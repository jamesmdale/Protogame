#include "Game\GameObjects\World.hpp"
#include "Game\Game.hpp"
#include "Game\GameStates\PlayingState.hpp"
#include "Game\ObjectMeshbuilder.hpp"
#include "Game\GameCommon.hpp"
#include "Game\Helpers\Ray.hpp"
#include "Game\Helpers\GameRendererHelpers.hpp"
#include "Game\Helpers\ChunkFileLoader.hpp"
#include "Game\Definitions\BlockDefinition.hpp"
#include "Game\GameObjects\Block.hpp"
#include "Game\GameObjects\BlockLocator.hpp"
#include "Game\GameObjects\Player.hpp"
#include "Engine\Window\Window.hpp"
#include "Engine\Debug\DebugRender.hpp"
#include "Engine\Core\LightObject.hpp"
#include "Engine\Debug\DebugRender.hpp"
#include "Engine\Renderer\RenderScene.hpp"
#include "Engine\Renderer\RenderScene2D.hpp"
#include "Engine\Core\StringUtils.hpp"
#include "Engine\Math\IntVector2.hpp"
#include "Engine\Math\MathUtils.hpp"
#include "Engine\Renderer\RendererTypes.hpp"
#include "Engine\Renderer\Material.hpp"
#include "Engine\Renderer\Shader.hpp"
#include "Engine\Renderer\ShaderProgram.hpp"
#include "Engine\Math\RawNoise.hpp"
#include "Engine\Math\SmoothNoise.hpp"
#include <map>
#include <string>
#include <algorithm>

//  =========================================================================================
World::World()
{

}

//  =========================================================================================
World::~World()
{
	//these will get deleted from game later becuase it's used in other gamestates
	m_engineCamera = nullptr;
	m_uiCamera = nullptr;

	delete(m_gameCamera);
	m_gameCamera = nullptr;

	delete(m_debugSkyMesh);
	m_debugSkyMesh = nullptr;
}

//  =========================================================================================
void World::Initialize()
{
	//initialize game camera
	Renderer* theRenderer = Renderer::GetInstance();

	m_engineCamera = Game::GetInstance()->m_engineCamera;
	m_uiCamera = Game::GetInstance()->m_uiCamera;

	m_gameCamera = new GameCamera();
	m_gameCamera->m_position = Vector3(0.1f, 0.1f, 110.f);

	//initialize entities
	m_player = new Player(this);
	m_player->m_position = Vector3(0.f, 0.f, 115.f);

	m_gameCamera->AttachToEntity(m_player);

	//m_camera->m_skybox = new Skybox("Data/Images/galaxy2.png");
	theRenderer->SetAmbientLightIntensity(0.15f);

	GenerateChunkBuildOrderCheatSheet();
	LoadSavedChunkReferences();

	theRenderer->SetLineWidth(3.f);

	InitializeSelectableBlockList();

	m_globalIndoorLightColor = g_defaultIndoorLightColor;
	m_globalOutdoorLightColor = g_defaultOutdoorLightColor;
	m_skyColor = g_lightBlue;	
  
	m_fogNearFarRange.y = (CHUNK_DISTANCE_RENDER * 16.f) - 48.f; //fog far
	m_fogNearFarRange.x = m_fogNearFarRange.y * 0.35f; //fog near
}

//  =========================================================================================
void World::Update(float deltaSeconds)
{
	//input & time
	m_player->UpdateFromInput(deltaSeconds);
	m_gameCamera->UpdateFromInput(deltaSeconds);
	UpdateFromInput(deltaSeconds);
	UpdateTime(deltaSeconds);

	UpdateGlobalLightingColors();

	//chunks
	ActivateChunks();
	UpdateDirtyLighting();
	GenerateDirtyChunks();
	DeactivateChunks();

	//camera and player
	/*if (m_activeChunks.size() > 3)
	{*/
	UpdateEntities(deltaSeconds);

	//update physics
	UpdateEntityPhysics(deltaSeconds);

	//player raycast
	if (m_gameCamera->m_currentCameraMode != DETACHED_CAMERA_MODE)
	{
		UpdateCameraViewPosition();		
	}

	Vector3 playerEyePosition = m_player->m_firstPersonCameraPositionOffsetFromPivot + m_player->GetBottomCenterPivot();
	m_raycastResult = Raycast(playerEyePosition, m_player->GetForward(), RAYCAST_MAX_DISTANCE);
			
	//}		
}

//  =========================================================================================
void World::PreRender()
{
	m_player->PreRender();
}

//  =========================================================================================
void World::Render()
{
	Renderer* theRenderer = Renderer::GetInstance();

	//always do this first at the beginning of the frame's render
	theRenderer->SetCamera(m_engineCamera);	
	theRenderer->ClearDepth(1.f);
	theRenderer->ClearColor(m_skyColor);

	//hammer over camera view matrix
	m_gameCamera->CreateFliippedViewMatrix(m_engineCamera->m_viewMatrix);

	//render
	RenderChunks();
	RenderEntities();
	RenderDebug();
	RenderUI();

	//theRenderer->ClearColor(Rgba(0.5f, 0.5f, 1.f, 1.f));
}

//  =========================================================================================
void World::UpdateFromInput(float deltaSeconds)
{
	InputSystem* theInput = InputSystem::GetInstance();
	Game* theGame = Game::GetInstance();

	Vector2 mouseDelta = Vector2::ZERO;
	mouseDelta = InputSystem::GetInstance()->GetMouse()->GetMouseDelta();

	//reset chunks
	if (theInput->WasKeyJustPressed(theInput->KEYBOARD_U) && theGame->m_inputDelayTimer->HasElapsed())
	{
		DeactivateAllChunks();
	}

	//dig block
	if (theInput->WasKeyJustPressed(theInput->MOUSE_LEFT_CLICK) && theGame->m_inputDelayTimer->HasElapsed())
	{
		DigBlock();
		theGame->m_inputDelayTimer->Reset();
	}

	//place block
	if (theInput->WasKeyJustPressed(theInput->MOUSE_RIGHT_CLICK) && theGame->m_inputDelayTimer->HasElapsed())
	{
		PlaceBlock();
		theGame->m_inputDelayTimer->Reset();
	}

	//world time pause
	if (theInput->WasKeyJustPressed(theInput->KEYBOARD_P) && theGame->m_inputDelayTimer->HasElapsed())
	{
		m_isWorldTimePaused ? m_isWorldTimePaused = false : m_isWorldTimePaused = true;
	}

	//accelerate time
	if (theInput->IsKeyPressed(theInput->KEYBOARD_T))
	{
		m_worldTimeScale = 10000.f;
	}
	if (theInput->WasKeyJustReleased(theInput->KEYBOARD_T))
	{
		m_worldTimeScale = 200.f;
	}

	// debug keys ----------------------------------------------

	//toggle debug for sky blocks
	if (theInput->WasKeyJustPressed(theInput->KEYBOARD_1) && theGame->m_inputDelayTimer->HasElapsed())
	{
		if (m_debugSkyMesh == nullptr)
		{
			GenerateDebugSkyMesh();
		}
		else
		{
			delete(m_debugSkyMesh);
			m_debugSkyMesh = nullptr;
		}
			
		theGame->m_inputDelayTimer->Reset();
	}

	//refresh debug sky mesh
	if (theInput->WasKeyJustPressed(theInput->KEYBOARD_K) && theGame->m_inputDelayTimer->HasElapsed())
	{
		if(m_debugSkyMesh != nullptr)
			GenerateDebugSkyMesh();

		theGame->m_inputDelayTimer->Reset();
	}

	//enable debug for dirty lighting
	if (theInput->WasKeyJustPressed(theInput->KEYBOARD_2) && theGame->m_inputDelayTimer->HasElapsed())
	{
		m_isDebugDirtyLighting ? m_isDebugDirtyLighting = false : m_isDebugDirtyLighting = true;
		theGame->m_inputDelayTimer->Reset();
	}

	//enable debug for dirty lighting
	if (theInput->WasKeyJustPressed(theInput->KEYBOARD_3) && theGame->m_inputDelayTimer->HasElapsed())
	{
		m_isDebugRGB ? m_isDebugRGB = false : m_isDebugRGB = true;
		theGame->m_inputDelayTimer->Reset();
	}	

	//enable debug for sky blocks
	if (theInput->WasKeyJustPressed(theInput->KEYBOARD_L) && theGame->m_inputDelayTimer->HasElapsed())
	{
		m_shouldStepDirtyLightingDebug = true;
		theGame->m_inputDelayTimer->Reset();
	}

	//lighting enabled/disabled
	if (theInput->WasKeyJustPressed(theInput->KEYBOARD_4) && theGame->m_inputDelayTimer->HasElapsed())
	{
		m_lightingEffectsDisabled ? m_lightingEffectsDisabled = false : m_lightingEffectsDisabled = true;
	}

	//mouse wheel scrolling for changing blocks
	if ((theInput->GetMouseWheelUp() || theInput->GetMouseWheelDown()) && theGame->m_inputDelayTimer->HasElapsed())
	{
		if (theInput->GetMouseWheelUp())
		{
			m_selectedBlockIndex++;
			m_selectedBlockIndex = m_selectedBlockIndex % ((int)m_selectableBlockTypes.size());
		}
		else if (theInput->GetMouseWheelDown())
		{
			m_selectedBlockIndex--;
			if(m_selectedBlockIndex < 0)
				m_selectedBlockIndex = UINT8_MAX;

			m_selectedBlockIndex = m_selectedBlockIndex % ((int)m_selectableBlockTypes.size());
		}
	}

	//exit player position and freelook
	if (theInput->WasKeyJustPressed(theInput->KEYBOARD_PAGEUP) && theGame->m_inputDelayTimer->HasElapsed())
	{
		ToggleCameraViewLocked();
		theGame->m_inputDelayTimer->Reset();
	}

	if (theInput->WasKeyJustPressed(theInput->KEYBOARD_R))
	{
		m_player->m_position = m_player->m_position + Vector3(0.f, 0.f, 15.f);
		m_player->m_velocity = Vector3::ZERO;
		m_player->UpdateBoundsToCurrentPosition();
	}

	//change camera modes
	if (theInput->WasKeyJustPressed(theInput->KEYBOARD_F2) && theGame->m_inputDelayTimer->HasElapsed())
	{
		m_gameCamera->CycleCameraModes();
		theGame->m_inputDelayTimer->Reset();
	}

	//change physics mode
	if (theInput->WasKeyJustPressed(theInput->KEYBOARD_F3) && theGame->m_inputDelayTimer->HasElapsed())
	{
		m_player->CyclePhysicsModes();
		theGame->m_inputDelayTimer->Reset();
	}

	if(theInput->WasKeyJustPressed(theInput->KEYBOARD_ESCAPE))
	{
		DeactivateAllChunks();
		g_isQuitting = true;
	}

	//deltaSeconds = m_player->UpdateFromInput(deltaSeconds);
	//deltaSeconds = UpdateFromInputDebug(deltaSeconds);

	// return 
	//return deltaSeconds; //new deltaSeconds

}

//  =========================================================================================
void World::UpdateCameraViewPosition()
{
	if (!IsCameraViewLocked())
	{
		Vector3 cameraForward = Vector3(CosDegrees(m_gameCamera->m_yawDegreesZ) * CosDegrees(m_gameCamera->m_pitchDegreesY)
			, SinDegrees(m_gameCamera->m_yawDegreesZ) * CosDegrees(m_gameCamera->m_pitchDegreesY),
			-1.f * SinDegrees(m_gameCamera->m_pitchDegreesY));

		/*Vector3 modifiedPosition = Vector3::ZERO;
		if (m_gameCamera->m_currentCameraMode == THIRD_PERSON_CAMERA_MODE)
			modifiedPosition = (-1.f * cameraForward * 4.f);*/

		CopyCameraDataToPlayerView(m_gameCamera->m_position, cameraForward);
	}	
}

//  =========================================================================================
void World::UpdateDirtyLighting()
{
	//process until no blocks are dirty
	if (!m_isDebugDirtyLighting)
	{
		m_dirtyDebugLightingPoints.clear();
		while (m_blocksWithDirtyLighting.size() > 0)
		{
			BlockLocator blockToProcess = m_blocksWithDirtyLighting.front();
			m_blocksWithDirtyLighting.pop_front();

			ProcessLightingForBlock(blockToProcess);
		}
	}
	else if(m_isDebugDirtyLighting && m_shouldStepDirtyLightingDebug)
	{
		m_dirtyDebugLightingPoints.clear();
		std::deque<BlockLocator> blocksToProcessThisFrame;
		blocksToProcessThisFrame.swap(m_blocksWithDirtyLighting);
		int blocksDirtyBefore = (int)blocksToProcessThisFrame.size();

		while (blocksToProcessThisFrame.size() > 0)
		{
			BlockLocator blockToProcess = blocksToProcessThisFrame.front();
			blocksToProcessThisFrame.pop_front();

			ProcessLightingForBlock(blockToProcess);
		}

		m_shouldStepDirtyLightingDebug = false;
	}
}

//  =========================================================================================
void World::UpdateChunks()
{
	std::map<IntVector2, Chunk*>::iterator chunkIterator;

	for (chunkIterator = m_activeChunks.begin(); chunkIterator != m_activeChunks.end(); ++chunkIterator)
	{
		chunkIterator->second->Update();
	}
}

//  =========================================================================================
void World::UpdateEntities(float deltaSeconds)
{
	m_player->Update(deltaSeconds);
}

//  =========================================================================================
void World::UpdateTime(float deltaSeconds)
{
	if (!m_isWorldTimePaused)
	{
		m_currentTimeOfDay += deltaSeconds * m_worldTimeScale;

		//case for rollover
		if (m_currentTimeOfDay > TIME_PER_DAY_IN_SECONDS)
		{
			m_days++;
			m_currentTimeOfDay = fmodf(m_currentTimeOfDay, TIME_PER_DAY_IN_SECONDS);
		}	
	}
}

//  =========================================================================================
void World::UpdateGlobalLightingColors()
{
	m_globalOutdoorLightColor = g_defaultOutdoorLightColor;
	m_globalIndoorLightColor = g_defaultIndoorLightColor;

	UpdateLightingFromTimeOfDay();

	if (!m_lightingEffectsDisabled)
	{
		PerlinLightningStrike();
		IndoorLightingFlicker();
	}
}

//  =========================================================================================
void World::UpdateLightingFromTimeOfDay()
{
	float percentageInDay = m_currentTimeOfDay / TIME_PER_DAY_IN_SECONDS;

	//night time
	if (percentageInDay < SIX_AM_TIME_IN_DAY || percentageInDay > SIX_PM_TIME_IN_DAY)
	{
		m_skyColor = g_nightLightColor;
		m_globalOutdoorLightColor = g_minOutdoorLightColor;
	}

	//morning
	else if (percentageInDay > SIX_AM_TIME_IN_DAY && percentageInDay <= NOON_TIME_IN_DAY)
	{
		float fraction = (percentageInDay - SIX_AM_TIME_IN_DAY) / (NOON_TIME_IN_DAY - SIX_AM_TIME_IN_DAY);
		m_skyColor = Interpolate(g_nightLightColor, g_dayLightColor, fraction);
		m_globalOutdoorLightColor = Interpolate(g_minOutdoorLightColor, g_defaultOutdoorLightColor, fraction);
	}

	//evening
	else if (percentageInDay > NOON_TIME_IN_DAY && percentageInDay <= SIX_PM_TIME_IN_DAY)
	{
		float fraction = (percentageInDay - NOON_TIME_IN_DAY) / (SIX_PM_TIME_IN_DAY - NOON_TIME_IN_DAY);
		m_skyColor = Interpolate(g_dayLightColor, g_nightLightColor, fraction);
		m_globalOutdoorLightColor = Interpolate(g_defaultOutdoorLightColor, g_minOutdoorLightColor, fraction);
	}
}

//  =========================================================================================
void World::UpdateEntityPhysics(float deltaSeconds)
{
	m_physicsFPS = 1.f / deltaSeconds;
	m_player->UpdatePhysics(deltaSeconds);

	//other entities go here
}

//  =========================================================================================
void World::RenderUI()
{
	Renderer* theRenderer = Renderer::GetInstance();
	theRenderer->SetCamera(m_uiCamera);

	Mesh* texturedUIMesh = CreateTexturedUIMesh();
	if (texturedUIMesh != nullptr)
	{
		theRenderer->BindMaterial(theRenderer->CreateOrGetMaterial("default"));
		theRenderer->SetTexture(*GetTerrainSprites()->GetSpriteSheetTexture());
		theRenderer->m_defaultShader->SetFrontFace(WIND_COUNTER_CLOCKWISE);

		theRenderer->DrawMesh(texturedUIMesh);
		delete(texturedUIMesh);
	}

	//handle text
	Mesh* textMesh = CreateUITextMesh();
	if (textMesh != nullptr)
	{
		theRenderer->BindMaterial(theRenderer->CreateOrGetMaterial("text"));
		theRenderer->DrawMesh(textMesh);
		delete(textMesh);
	}
}

//  =========================================================================================
void World::RenderEntities()
{
	m_player->Render();

	//render other entities
}

//  =========================================================================================
void World::RenderChunks()
{
	Renderer* theRenderer = Renderer::GetInstance();
	
	//bind material
	if (m_isDebugRGB)
	{
		theRenderer->BindMaterial(theRenderer->CreateOrGetMaterial("default"));
	}
	else
	{
		Material* overworldOpaqueMaterial = theRenderer->CreateOrGetMaterial("OverworldOpaque");
		theRenderer->BindMaterial(overworldOpaqueMaterial);
		int shaderProgramHandle = overworldOpaqueMaterial->m_shader->m_program->GetHandle();

		//set material properties
		bool bindSuccess1 = theRenderer->SetVector3Uniform(shaderProgramHandle, "CAMERA_WORLD_POSITION", m_gameCamera->m_position);
		bool bindSuccess2 = theRenderer->SetVector3Uniform(shaderProgramHandle, "GLOBAL_INDOOR_LIGHT_COLOR", Rgba::ConvertToVector3(m_globalIndoorLightColor));
		bool bindSuccess3 = theRenderer->SetVector3Uniform(shaderProgramHandle, "GLOBAL_OUTDOOR_LIGHT_COLOR", Rgba::ConvertToVector3(m_globalOutdoorLightColor));
		bool bindSuccess4 = theRenderer->SetVector3Uniform(shaderProgramHandle, "SKY_COLOR", Rgba::ConvertToVector3(m_skyColor));
		bool bindSuccess5 = theRenderer->SetVector2Uniform(shaderProgramHandle, "NEAR_FAR_FOG_DISTANCE", m_fogNearFarRange);

		//ASSERT_OR_DIE(bindSuccess1, "BINDING FOR OVERWORLD SHADER BROKEN!!!");
		//ASSERT_OR_DIE(bindSuccess2, "BINDING FOR OVERWORLD SHADER BROKEN!!!");
		//ASSERT_OR_DIE(bindSuccess3, "BINDING FOR OVERWORLD SHADER BROKEN!!!");
		//ASSERT_OR_DIE(bindSuccess4, "BINDING FOR OVERWORLD SHADER BROKEN!!!");
		//ASSERT_OR_DIE(bindSuccess5, "BINDING FOR OVERWORLD SHADER BROKEN!!!");
	}

	theRenderer->SetTexture(*GetTerrainSprites()->GetSpriteSheetTexture());
	theRenderer->m_defaultShader->SetFrontFace(WIND_COUNTER_CLOCKWISE);

	//draw each chunk
	std::map<IntVector2, Chunk*>::iterator chunkIterator;
	for (chunkIterator = m_activeChunks.begin(); chunkIterator != m_activeChunks.end(); ++chunkIterator)
	{
		Chunk* chunk = chunkIterator->second;
		if(chunk->m_gpuMesh != nullptr)
			chunk->Render();
	}
}

//  =========================================================================================
void World::RenderDebug()
{
	Renderer* theRenderer = Renderer::GetInstance();

	// debug drawing ----------------------------------------------
	MeshBuilder builder;

	builder.CreateBasis(Matrix44::IDENTITY, Vector3::ZERO, 1.f);
	Mesh* axisMesh = builder.CreateMesh<VertexPCU>();

	//draw axis for debugging
	theRenderer->SetTexture(*theRenderer->CreateOrGetTexture("default"));
	theRenderer->BindMaterial(theRenderer->CreateOrGetMaterial("default_always_depth"));
	theRenderer->DrawMesh(axisMesh);	

	theRenderer->BindMaterial(theRenderer->CreateOrGetMaterial("default"));
	theRenderer->DrawMesh(axisMesh);

	Mesh* raycastBlockHighlightMesh = nullptr;
	Mesh* raycastMesh = nullptr;

	//draw raycastline
	raycastMesh = CreateLine(m_raycastResult.m_ray.m_startPosition, m_raycastResult.m_impactWorldPosition, Rgba::BLUE);
	theRenderer->SetTexture(*theRenderer->CreateOrGetTexture("default"));
	theRenderer->DrawMesh(raycastMesh);

	delete(raycastMesh);
	raycastMesh = nullptr;

	raycastMesh = CreateLine(m_raycastResult.m_impactWorldPosition, m_raycastResult.m_endPosition, Rgba::PINK);
	theRenderer->SetTexture(*theRenderer->CreateOrGetTexture("default"));
	theRenderer->DrawMesh(raycastMesh);

	delete(raycastMesh);
	raycastMesh = nullptr;

	raycastMesh = CreateDebugStar(m_raycastResult.m_impactWorldPosition, Rgba::BLUE, 0.05f);
	theRenderer->SetTexture(*theRenderer->CreateOrGetTexture("default"));
	theRenderer->DrawMesh(raycastMesh);

	delete(raycastMesh);
	raycastMesh = nullptr;

	raycastMesh = CreateDebugStar(m_raycastResult.m_endPosition, Rgba::PINK, 0.05f);
	theRenderer->SetTexture(*theRenderer->CreateOrGetTexture("default"));
	theRenderer->DrawMesh(raycastMesh);

	delete(raycastMesh);
	raycastMesh = nullptr;

	if (m_debugSkyMesh != nullptr)
	{
		theRenderer->SetTexture(*theRenderer->CreateOrGetTexture("default"));
		theRenderer->DrawMesh(m_debugSkyMesh);
	}

	if (m_isDebugDirtyLighting)
	{
		MeshBuilder debugLightBuilder;
		for (int debugLightPoint = 0; debugLightPoint < (int)m_dirtyDebugLightingPoints.size(); ++debugLightPoint)
		{
			CreateDebugStar(debugLightBuilder, m_dirtyDebugLightingPoints[debugLightPoint], Rgba::YELLOW, 0.25f);
		}

		Mesh* debugLightRenderMesh = debugLightBuilder.CreateMesh<VertexPCU>();
		theRenderer->SetTexture(*theRenderer->CreateOrGetTexture("default"));
		theRenderer->DrawMesh(debugLightRenderMesh);
		delete(debugLightRenderMesh);
		debugLightRenderMesh = nullptr;
	}

	/* NEIGHBORING BLOCK LOCATOR DEBUG =========================================================================================

	//IntVector2 chunkCoordsOfWorldPosition = IntVector2((int)floorf(m_raycastResult.m_ray.m_startPosition.x * BLOCKS_WIDE_X_DIVISOR), (int)floorf(m_raycastResult.m_ray.m_startPosition.y * BLOCKS_WIDE_Y_DIVISOR));

	//std::map<IntVector2, Chunk*>::iterator activeChunkIterator = m_activeChunks.find(chunkCoordsOfWorldPosition);
	//Chunk* currentRayChunk = activeChunkIterator->second;

	//uint outBlockIndex;
	//bool success = currentRayChunk->GetBlockIndexForWorldPositionWithinBounds(outBlockIndex, m_raycastResult.m_ray.m_startPosition);
	//
	////render me
	//BlockLocator locator = BlockLocator(currentRayChunk, outBlockIndex);
	//Vector3 worldCenter = currentRayChunk->GetBlockWorldCenterForBlockIndex(outBlockIndex);
	//raycastMesh = CreateBlockOutline(worldCenter, Rgba::WHITE);
	//theRenderer->SetTexture(*theRenderer->CreateOrGetTexture("default"));
	//theRenderer->DrawMesh(raycastMesh);

	//delete(raycastMesh);
	//raycastMesh = nullptr;

	////render above
	//BlockLocator aboveLocator = locator.GetBlockLocatorAbove();
	//if (aboveLocator.IsValid())
	//{	
	//	worldCenter = aboveLocator.m_chunk->GetBlockWorldCenterForBlockIndex(aboveLocator.m_blockIndex);
	//	raycastMesh = CreateBlockOutline(worldCenter, Rgba::BLUE);
	//	theRenderer->SetTexture(*theRenderer->CreateOrGetTexture("default"));
	//	theRenderer->DrawMesh(raycastMesh);

	//	delete(raycastMesh);
	//	raycastMesh = nullptr;
	//}

	//BlockLocator belowLocator = locator.GetBlockLocatorBelow();
	//if (belowLocator.IsValid())
	//{
	//	worldCenter = belowLocator.m_chunk->GetBlockWorldCenterForBlockIndex(belowLocator.m_blockIndex);
	//	raycastMesh = CreateBlockOutline(worldCenter, Rgba::LIGHT_BLUE);
	//	theRenderer->SetTexture(*theRenderer->CreateOrGetTexture("default"));
	//	theRenderer->DrawMesh(raycastMesh);

	//	delete(raycastMesh);
	//	raycastMesh = nullptr;
	//}
	//
	//BlockLocator eastLocator = locator.GetBlockLocatorToEast();
	//if (eastLocator.IsValid())
	//{
	//	worldCenter = eastLocator.m_chunk->GetBlockWorldCenterForBlockIndex(eastLocator.m_blockIndex);
	//	raycastMesh = CreateBlockOutline(worldCenter, Rgba::RED);
	//	theRenderer->SetTexture(*theRenderer->CreateOrGetTexture("default"));
	//	theRenderer->DrawMesh(raycastMesh);

	//	delete(raycastMesh);
	//	raycastMesh = nullptr;
	//}

	//BlockLocator westLocator = locator.GetBlockLocatorToWest();
	//if (westLocator.IsValid())
	//{
	//	worldCenter = westLocator.m_chunk->GetBlockWorldCenterForBlockIndex(westLocator.m_blockIndex);
	//	raycastMesh = CreateBlockOutline(worldCenter, Rgba::LIGHT_RED);
	//	theRenderer->SetTexture(*theRenderer->CreateOrGetTexture("default"));
	//	theRenderer->DrawMesh(raycastMesh);

	//	delete(raycastMesh);
	//	raycastMesh = nullptr;
	//}

	//BlockLocator northLocator = locator.GetBlockLocatorToNorth();
	//if (northLocator.IsValid())
	//{
	//	worldCenter = northLocator.m_chunk->GetBlockWorldCenterForBlockIndex(northLocator.m_blockIndex);
	//	raycastMesh = CreateBlockOutline(worldCenter, Rgba::GREEN);
	//	theRenderer->SetTexture(*theRenderer->CreateOrGetTexture("default"));
	//	theRenderer->DrawMesh(raycastMesh);

	//	delete(raycastMesh);
	//	raycastMesh = nullptr;
	//}

	//BlockLocator southLocator = locator.GetBlockLocatorToSouth();
	//if (southLocator.IsValid())
	//{
	//	worldCenter = southLocator.m_chunk->GetBlockWorldCenterForBlockIndex(southLocator.m_blockIndex);
	//	raycastMesh = CreateBlockOutline(worldCenter, Rgba::LIGHT_GREEN);
	//	theRenderer->SetTexture(*theRenderer->CreateOrGetTexture("default"));
	//	theRenderer->DrawMesh(raycastMesh);

	//	delete(raycastMesh);
	//	raycastMesh = nullptr;
	//}

	//  =========================================================================================
	*/

	//draw raycast block highlighting
	if (m_raycastResult.m_didImpact && m_raycastResult.m_impactBlockLocator.IsValid())
	{
		Vector3 blockCenter = m_raycastResult.m_impactBlockLocator.m_chunk->GetBlockWorldCenterForBlockIndex(m_raycastResult.m_impactBlockLocator.m_blockIndex);
		raycastBlockHighlightMesh = CreateBlockOutline(blockCenter, Rgba::WHITE, 1.01f);
		theRenderer->SetTexture(*theRenderer->CreateOrGetTexture("default"));
		theRenderer->DrawMesh(raycastBlockHighlightMesh);

		delete(raycastBlockHighlightMesh);
		raycastBlockHighlightMesh = nullptr;

		raycastBlockHighlightMesh = CreateBlockHighlightBox(blockCenter, m_raycastResult.m_impactNormal, 1.01f);
		theRenderer->SetTexture(*theRenderer->CreateOrGetTexture("default"));
		theRenderer->BindMaterial(theRenderer->CreateOrGetMaterial("default"));
		theRenderer->DrawMesh(raycastBlockHighlightMesh);
		theRenderer->BindMaterial(theRenderer->CreateOrGetMaterial("default"));

		delete(raycastBlockHighlightMesh);
		raycastBlockHighlightMesh = nullptr;
	}	

	//cleanup
	TODO("later we need to just put these in the world or above it in the game so we aren't newing");
	delete(axisMesh);
	axisMesh = nullptr;

	if (raycastMesh != nullptr)
	{
		delete(raycastMesh);
		raycastMesh = nullptr;
	}

	if (raycastBlockHighlightMesh != nullptr)
	{
		delete(raycastBlockHighlightMesh);
		raycastBlockHighlightMesh = nullptr;
	}	
}

//  =========================================================================================
void World::ActivateChunks()
{
	IntVector2 playerChunkCoords = IntVector2(floorf(m_gameCamera->m_position.x * BLOCKS_WIDE_X_DIVISOR), floorf(m_gameCamera->m_position.y * BLOCKS_WIDE_Y_DIVISOR));
	Vector2 playerChunkCenter = Vector2(playerChunkCoords) + Vector2(0.5f, 0.5f);

	//loop through my neighborhood and find the first chunk that needs activating(loaded or generated, hooked up)
	for (int chunkIndex = 0; chunkIndex < (int)m_neighborHoodBuildOrder.size(); ++chunkIndex)
	{
		IntVector2 worldChunkCoord = playerChunkCoords + m_neighborHoodBuildOrder[chunkIndex];
		std::map<IntVector2, Chunk*>::iterator chunkIterator =  m_activeChunks.find(worldChunkCoord);

		if (chunkIterator == m_activeChunks.end())
		{
			float distanceOut = 0.f;
			//we didn't find the chunk in the activation list so we must confirm we should add it
			if (CompareDistanceFromLocationToLocationLessThanRadius(distanceOut, playerChunkCenter, playerChunkCenter + Vector2(m_neighborHoodBuildOrder[chunkIndex]), (float)CHUNK_DISTANCE_ACTIVATION))
			{
				ActivateChunk(worldChunkCoord);

				//for now we are only activating one chunk a frame
				return;
			}
			else
			{
				continue;
			}
		}	
	}
}

////  =========================================================================================
//void World::MarkUnrenderedChunksDirtyWithinRenderRadius()
//{
//	IntVector2 playerChunkCoords = IntVector2(floorf(m_gameCamera->m_position.x / BLOCKS_WIDE_X), floorf(m_gameCamera->m_position.y / BLOCKS_WIDE_Y));
//	Vector2 playerChunkCenter = Vector2(playerChunkCoords) + Vector2(0.5f, 0.5f);
//
//	std::map<IntVector2, Chunk*>::iterator activeChunkIterator;
//	for (activeChunkIterator = m_activeChunks.begin(); activeChunkIterator != m_activeChunks.end(); ++activeChunkIterator)
//	{
//		//we the chunk doesn't have a mesh yet, we should check to see if need to draw it.
//		Chunk* chunk = activeChunkIterator->second;
//		
//		//we've already got a mesh for them
//		if(chunk->m_gpuMesh != nullptr)
//			continue;
//
//		float distanceOut = 0.f;
//		if (chunk->DoesHaveAllNeighbors() && CompareDistanceFromLocationToLocationLessThanRadius(distanceOut, playerChunkCenter, Vector2(activeChunkIterator->first) + Vector2(0.5f, 0.5f), (float)CHUNK_DISTANCE_RENDER))
//		{
//			chunk->m_isMeshDirty = true;
//			return;
//		}
//	}
//}

//  =========================================================================================
void World::GenerateDirtyChunks()
{
	IntVector2 playerChunkCoords = IntVector2(floorf(m_gameCamera->m_position.x * BLOCKS_WIDE_X_DIVISOR), floorf(m_gameCamera->m_position.y * BLOCKS_WIDE_Y_DIVISOR));
	Vector2 playerChunkCenter = Vector2(playerChunkCoords) + Vector2(0.5f, 0.5f);

	//loop through my active chunk list and generate the mesh for anyone marked as dirty
	std::map<IntVector2, Chunk*>::iterator activeChunkIterator;
	for (activeChunkIterator = m_activeChunks.begin(); activeChunkIterator != m_activeChunks.end(); ++activeChunkIterator)
	{
		Chunk* chunk = activeChunkIterator->second;
		float distanceOut = 0.f;
		if (chunk->m_isMeshDirty && chunk->DoesHaveAllNeighbors() && CompareDistanceFromLocationToLocationLessThanRadius(distanceOut, playerChunkCenter, Vector2(activeChunkIterator->first) + Vector2(0.5f, 0.5f), (float)CHUNK_DISTANCE_RENDER))
		{
			chunk->GenerateChunkMesh();
			chunk->m_isMeshDirty = false;
		}
	}
}

//  =========================================================================================
void World::DeactivateChunks()
{
	//loop through my active chunk list and determine find the first active chunk out of range to be deactivated(saved if needed, unhooked, deleted)

	IntVector2 playerChunkCoords = IntVector2(floorf(m_gameCamera->m_position.x * BLOCKS_WIDE_X_DIVISOR), floorf(m_gameCamera->m_position.y * BLOCKS_WIDE_Y_DIVISOR));
	Vector2 playerChunkCenter = Vector2(playerChunkCoords) + Vector2(0.5f, 0.5f);

	//loop through my neighborhood and find the first chunk that needs activating(loaded or generated, hooked up)
	std::map<IntVector2, Chunk*>::iterator furthestAwayChunkIterator = m_activeChunks.end();
	float furthestDistance = 0.f;

	std::map<IntVector2, Chunk*>::iterator activeChunkIterator;
	for (activeChunkIterator = m_activeChunks.begin(); activeChunkIterator != m_activeChunks.end(); ++activeChunkIterator)
	{
		//we didn't find the chunk in the activation list so we must confirm we should add it
		float distanceOut = 0.f;
		if (CompareDistanceFromLocationToLocationGreaterThanRadius(distanceOut, playerChunkCenter, Vector2(activeChunkIterator->first) + Vector2(0.5f, 0.5f), (float)CHUNK_DISTANCE_DEACTIVATION))
		{
			//store the chunk that is the furthest away
			if (distanceOut > furthestDistance)
			{
				furthestAwayChunkIterator = activeChunkIterator;
			}
		}			
	}

	if (furthestAwayChunkIterator != m_activeChunks.end())
	{
		DeactivateChunk(furthestAwayChunkIterator->first);
	}
}

//  =========================================================================================
void World::DeactivateAllChunks()
{
	std::map<IntVector2, Chunk*>::iterator activeChunkIterator = m_activeChunks.begin();
	while(m_activeChunks.size() > 0)
	{
		DeactivateChunk(activeChunkIterator->first);
		activeChunkIterator = m_activeChunks.begin();
	}
}

//  =========================================================================================
void World::ActivateChunk(const IntVector2& chunkCoordinates)
{
	Chunk* chunk = new Chunk(chunkCoordinates);
	m_activeChunks.insert(std::pair<IntVector2, Chunk*>(chunk->m_chunkCoords, chunk));

	bool success = ReadChunkDataFromFile(chunk);

	if (!success)
	{
		//randomization
		chunk->GenerateBlockDataWithPerlin();
	}
	else
	{
		int i = 0;
	}

	// hook neighbors if they exist ----------------------------------------------
	std::map<IntVector2, Chunk*>::iterator activeChunkIterator;

	//find north neighbor and hook them if they exist
	activeChunkIterator = m_activeChunks.find(chunkCoordinates + g_chunkNorth);
	if (activeChunkIterator != m_activeChunks.end())
	{
		chunk->AddNeighbor(activeChunkIterator->second, NORTH_NEIGHBOR_TYPE);
	}

	//find west neighbor and hook them if they exist
	activeChunkIterator = m_activeChunks.find(chunkCoordinates + g_chunkWest);
	if (activeChunkIterator != m_activeChunks.end())
	{
		chunk->AddNeighbor(activeChunkIterator->second, WEST_NEIGHBOR_TYPE);
	}

	//find south neighbor and hook them if they exist
	activeChunkIterator = m_activeChunks.find(chunkCoordinates + g_chunkSouth);
	if (activeChunkIterator != m_activeChunks.end())
	{
		chunk->AddNeighbor(activeChunkIterator->second, SOUTH_NEIGHBOR_TYPE);
	}

	//find east neighbor and hook them if they exist
	activeChunkIterator = m_activeChunks.find(chunkCoordinates + g_chunkEast);
	if (activeChunkIterator != m_activeChunks.end())
	{
		chunk->AddNeighbor(activeChunkIterator->second, EAST_NEIGHBOR_TYPE);
	}

	chunk->m_isMeshDirty = true;

	//handle lighting
	InitializeChunkLighting(chunk);
}

//  =========================================================================================
void World::InitializeChunkLighting(Chunk* chunk)
{
	ASSERT_OR_DIE(chunk != nullptr, "INVALID CHUNK PASSED TO 'void World::InitializeChunkLighting()'!!!");

	//label sky blocks for each column  ----------------------------------------------
	for (int columnIndex = 0; columnIndex < BLOCKS_PER_LAYER; ++columnIndex)
	{
		bool isBlockSky = true;
		//get highest block in column
		IntVector3 blockCoords = chunk->GetBlockCoordsForBlockIndex(columnIndex);
		blockCoords.z = BLOCKS_HIGH_Z - 1; //change index to the highest in the column

		int blockIndex = chunk->GetBlockIndexForBlockCoords(blockCoords);
		BlockLocator currentBlockLocator = BlockLocator(chunk, blockIndex);

		while (isBlockSky)
		{
			//get the block locator below
			Block* blockToProcess = currentBlockLocator.GetBlock();

			//if block is visible we are at the 'ground' (first non air/sky block) at the current index
			if (blockToProcess->IsFullOpaque() || !blockToProcess->IsValid())
			{
				isBlockSky = false;
			}
			else
			{
				blockToProcess->SetSkyFlag(true);
				currentBlockLocator.StepDown();	
			}					
		}
	}

	//mark sky neighbors dirty and set max outdoor lighting for sky blocks  ----------------------------------------------
	for (int columnIndex = 0; columnIndex < BLOCKS_PER_LAYER; ++columnIndex)
	{
		bool isBlockSky = true;
		//get highest block in column
		IntVector3 blockCoords = chunk->GetBlockCoordsForBlockIndex(columnIndex);
		blockCoords.z = BLOCKS_HIGH_Z - 1; //change index to the highest in the column

		int blockIndex = chunk->GetBlockIndexForBlockCoords(blockCoords);
		BlockLocator currentBlockLocator = BlockLocator(chunk, blockIndex);

		while (isBlockSky)
		{
			//get the block locator below
			Block* blockToProcess = currentBlockLocator.GetBlock();

			//if block is visible we are at the 'ground' (first non air/sky block) at the current index
			if (!blockToProcess->IsSky())
			{
				isBlockSky = false;
			}
			else
			{			
				blockToProcess->SetOutdoorLightingValue(MAX_OUTDOOR_LIGHTING_VALUE);
				//mark cardinal neighbors that AREN'T SKY dirty

				//north block
				BlockLocator northBlockLocator = currentBlockLocator.GetBlockLocatorToNorth();
				Block* northBlock = northBlockLocator.GetBlock();
				if(!northBlock->IsSky() && !northBlock->IsFullOpaque())
					AddBlockLocatorToDirtyLightingQueue(northBlockLocator);

				//south block
				BlockLocator southBlockLocator = currentBlockLocator.GetBlockLocatorToSouth();
				Block* southBlock = southBlockLocator.GetBlock();
				if(!southBlock->IsSky() && !southBlock->IsFullOpaque())
					AddBlockLocatorToDirtyLightingQueue(southBlockLocator);

				//east block
				BlockLocator eastBlockLocator = currentBlockLocator.GetBlockLocatorToEast();
				Block* eastBlock = eastBlockLocator.GetBlock();
				if(!eastBlock->IsSky() && !eastBlock->IsFullOpaque())
					AddBlockLocatorToDirtyLightingQueue(eastBlockLocator);

				//west block
				BlockLocator westBlockLocator = currentBlockLocator.GetBlockLocatorToWest();
				Block* westBlock = westBlockLocator.GetBlock();
				if(!westBlock->IsSky() && !westBlock->IsFullOpaque())
					AddBlockLocatorToDirtyLightingQueue(westBlockLocator);

				//set block locator to block locator below
				currentBlockLocator.StepDown();
			}
		}
	}
	
	//mark all light sources dirty in the chunk  ----------------------------------------------
	for (int blockIndex = 0; blockIndex < BLOCKS_PER_CHUNK; ++blockIndex)
	{
		BlockLocator block = BlockLocator(chunk, blockIndex);

		//if the block emits light, mark it as dirty
		if (chunk->m_blocks[blockIndex].DoesEmitLight())
		{
			AddBlockLocatorToDirtyLightingQueue(block);
		}

		//if the block is on an edge and is not visible, mark it as dirty
		if (block.IsBlockIndexOnEdgeFast() && !block.GetBlock()->IsVisible())
		{
			AddBlockLocatorToDirtyLightingQueue(block);
		}		
	}
}

//  =========================================================================================
void World::DeactivateChunk(const IntVector2& keyVal)
{
	//before deletion need to check saving and unhooking of neighbors

	Chunk* chunk = m_activeChunks.at(keyVal);

	if (chunk->m_doesRequireSave)
	{
		bool success = WriteChunkDataToFile(chunk);

		ASSERT_OR_DIE(success, "CHUNK SAVE INVALID");
	}

	chunk->UnhookNeighbors();

	delete(chunk);
	chunk = nullptr;

	m_activeChunks.erase(keyVal);
}

//  =========================================================================================
void World::GenerateChunkBuildOrderCheatSheet()
{
	float maxNumActiveBlocks = CHUNK_DISTANCE_DEACTIVATION;

	for (int chunkY = -maxNumActiveBlocks; chunkY <= maxNumActiveBlocks; ++chunkY)
	{
		for (int chunkX = -maxNumActiveBlocks; chunkX <= maxNumActiveBlocks; ++chunkX)
		{
			m_neighborHoodBuildOrder.push_back(IntVector2(chunkX, chunkY));
		}		
	}

	//now that we have all the indices we want relative to their distance from zero, let's sort them so we have our list
	std::sort(m_neighborHoodBuildOrder.begin(), m_neighborHoodBuildOrder.end(), CompareDistanceFromZeroLessThan);
}

//  =========================================================================================
void World::LoadSavedChunkReferences()
{
	TODO("Load chunks from start so we have a list");
}

//  =========================================================================================
BlockLocator World::GetChunkByPositionFromChunkList(const Vector3& worldPosition)
{
	BlockLocator locator;

	// get chunk information we need for  ----------------------------------------------
	IntVector2 chunkCoordsOfWorldPosition = IntVector2((int)floorf(worldPosition.x * BLOCKS_WIDE_X_DIVISOR), (int)floorf(worldPosition.y * BLOCKS_WIDE_Y_DIVISOR));

	std::map<IntVector2, Chunk*>::iterator activeChunkIterator = m_activeChunks.find(chunkCoordsOfWorldPosition);

	if (activeChunkIterator == m_activeChunks.end())
	{
		return locator;
	}

	Chunk* currentChunk = activeChunkIterator->second;

	uint outBlockIndex;
	bool success = currentChunk->GetBlockIndexForWorldPositionWithinBounds(outBlockIndex, worldPosition);

	//we are out of bounds of the chunk bounds
	if (!success)
	{
		return locator;
	}

	locator = BlockLocator(currentChunk, outBlockIndex);
	return locator;
}

//  =========================================================================================
void World::ProcessLightingForBlock(BlockLocator blockLocator)
{
	Block* block = blockLocator.GetBlock();

	//if this is the invalid block, do not process
	if(!block->IsValid())
		return;

	BlockDefinition* blockDef = BlockDefinition::GetDefinitionById(block->m_type);

	//get all neighbors ----------------------------------------------
	BlockLocator northNeighbor = blockLocator.GetBlockLocatorToNorth();
	Block* northBlock = northNeighbor.GetBlock();

	BlockLocator southNeighbor = blockLocator.GetBlockLocatorToSouth();
	Block* southBlock = southNeighbor.GetBlock();

	BlockLocator eastNeighbor = blockLocator.GetBlockLocatorToEast();
	Block* eastBlock = eastNeighbor.GetBlock();

	BlockLocator westNeighbor = blockLocator.GetBlockLocatorToWest();
	Block* westBlock = westNeighbor.GetBlock();

	BlockLocator aboveNeighbor = blockLocator.GetBlockLocatorAbove();
	Block* aboveBlock = aboveNeighbor.GetBlock();

	BlockLocator belowNeighbor = blockLocator.GetBlockLocatorBelow();
	Block* belowBlock = belowNeighbor.GetBlock();

	//remove the bit flag for the block saying it's dirty ----------------------------------------------
	block->SetLightingInDirtyListFlag(false);

	uint8 minIndoorLightingValue = blockDef->m_minimumLightingValue;
	uint8 minOutdoorLightingValue = 0;
	if(block->IsSky())
		minOutdoorLightingValue = MAX_OUTDOOR_LIGHTING_VALUE;

	uint8 currentIndoorLightingValue = block->GetIndoorLightingValue();
	uint8 currentOutDoorLightingValue = block->GetOutDoorLightingValue();
	uint8 newIndoorLightingValue = currentIndoorLightingValue;
	uint8 newOutDoorLightingValue = currentOutDoorLightingValue;

	uint8 brightestNeighborIndoorLightingValue = 0;
	uint8 brightestNeighborOutdoorLightingValue = 0;

	//process north neighbor ----------------------------------------------	
	if (northNeighbor.IsValid())
	{
		//get indoor lighting  level
		uint8 neighborIndoorLightingValue = northBlock->GetIndoorLightingValue();

		if(neighborIndoorLightingValue > brightestNeighborIndoorLightingValue)
			brightestNeighborIndoorLightingValue = neighborIndoorLightingValue;

		//get outdoor lighting level
		uint8 neighborOutdoorLightingValue = northBlock->GetOutDoorLightingValue();

		if(neighborOutdoorLightingValue > brightestNeighborOutdoorLightingValue)
			brightestNeighborOutdoorLightingValue = neighborOutdoorLightingValue;
	}

	//process south neighbor ----------------------------------------------
	if (southNeighbor.IsValid())
	{
		//get indoor lighting  level
		uint8 neighborIndoorLightingValue = southBlock->GetIndoorLightingValue();

		if(neighborIndoorLightingValue > brightestNeighborIndoorLightingValue)
			brightestNeighborIndoorLightingValue = neighborIndoorLightingValue;

		//get outdoor lighting level
		uint8 neighborOutdoorLightingValue = southBlock->GetOutDoorLightingValue();

		if(neighborOutdoorLightingValue > brightestNeighborOutdoorLightingValue)
			brightestNeighborOutdoorLightingValue = neighborOutdoorLightingValue;
	}

	//process east neighbor ----------------------------------------------
	if (eastNeighbor.IsValid())
	{
		//get indoor lighting  level
		uint8 neighborIndoorLightingValue = eastBlock->GetIndoorLightingValue();

		if(neighborIndoorLightingValue > brightestNeighborIndoorLightingValue)
			brightestNeighborIndoorLightingValue = neighborIndoorLightingValue;

		//get outdoor lighting level
		uint8 neighborOutdoorLightingValue = eastBlock->GetOutDoorLightingValue();

		if(neighborOutdoorLightingValue > brightestNeighborOutdoorLightingValue)
			brightestNeighborOutdoorLightingValue = neighborOutdoorLightingValue;
	}

	//process west neighbor ----------------------------------------------
	if (westNeighbor.IsValid())
	{
		//get indoor lighting  level
		uint8 neighborIndoorLightingValue = westBlock->GetIndoorLightingValue();

		if(neighborIndoorLightingValue > brightestNeighborIndoorLightingValue)
			brightestNeighborIndoorLightingValue = neighborIndoorLightingValue;

		//get outdoor lighting level
		uint8 neighborOutdoorLightingValue = westBlock->GetOutDoorLightingValue();

		if(neighborOutdoorLightingValue > brightestNeighborOutdoorLightingValue)
			brightestNeighborOutdoorLightingValue = neighborOutdoorLightingValue;
	}

	//process above neighbor ----------------------------------------------
	if (aboveNeighbor.IsValid())
	{
		//get indoor lighting level
		uint8 neighborIndoorLightingValue = aboveBlock->GetIndoorLightingValue();

		if(neighborIndoorLightingValue > brightestNeighborIndoorLightingValue)
			brightestNeighborIndoorLightingValue = neighborIndoorLightingValue;

		//get outdoor lighting level
		uint8 neighborOutdoorLightingValue = aboveBlock->GetOutDoorLightingValue();

		if(neighborOutdoorLightingValue > brightestNeighborOutdoorLightingValue)
			brightestNeighborOutdoorLightingValue = neighborOutdoorLightingValue;
	}

	// process below neighbor ----------------------------------------------
	if (belowNeighbor.IsValid())
	{
		//get indoor lighting level
		uint8 neighborIndoorLightingValue = belowBlock->GetIndoorLightingValue();

		if(neighborIndoorLightingValue > brightestNeighborIndoorLightingValue)
			brightestNeighborIndoorLightingValue = neighborIndoorLightingValue;

		//get outdoor lighting level
		uint8 neighborOutdoorLightingValue = belowBlock->GetOutDoorLightingValue();

		if(neighborOutdoorLightingValue > brightestNeighborOutdoorLightingValue)
			brightestNeighborOutdoorLightingValue = neighborOutdoorLightingValue;
	}

	//now that we have checked all the neighbors, change the value to 1 less than the highest neighbor ----------------------------------------------

	//indoor lighting check
	if ((brightestNeighborIndoorLightingValue - 1) > minIndoorLightingValue && (brightestNeighborIndoorLightingValue - 1) >= 0)
		newIndoorLightingValue = brightestNeighborIndoorLightingValue - 1;
	else
		newIndoorLightingValue = minIndoorLightingValue;

	//outdoor lighting check
	if ((brightestNeighborOutdoorLightingValue - 1) > minOutdoorLightingValue && (brightestNeighborOutdoorLightingValue - 1) >= 0)
		newOutDoorLightingValue = brightestNeighborOutdoorLightingValue - 1;
	else
		newOutDoorLightingValue = minOutdoorLightingValue;


	//if we have a change, update the value to the new lighting value and dirty my neighbors
	if (newIndoorLightingValue != currentIndoorLightingValue || newOutDoorLightingValue != currentOutDoorLightingValue)
	{
		if (!block->IsFullOpaque() || block->DoesEmitLight())
		{
			block->SetIndoorLightingValue(newIndoorLightingValue);
			block->SetOutdoorLightingValue(newOutDoorLightingValue);
			blockLocator.m_chunk->m_isMeshDirty = true;
		}	

		//foreach neighbor, if they AREN'T opaque, add them to the dirty list
		if(northBlock->IsValid() && !northBlock->IsFullOpaque())
			AddBlockLocatorToDirtyLightingQueue(northNeighbor);
		if(northBlock->IsValid() &&!southBlock->IsFullOpaque())
			AddBlockLocatorToDirtyLightingQueue(southNeighbor);
		if(northBlock->IsValid() &&!eastBlock->IsFullOpaque())
			AddBlockLocatorToDirtyLightingQueue(eastNeighbor);
		if(northBlock->IsValid() &&!westBlock->IsFullOpaque())
			AddBlockLocatorToDirtyLightingQueue(westNeighbor);
		if(northBlock->IsValid() &&!aboveBlock->IsFullOpaque())
			AddBlockLocatorToDirtyLightingQueue(aboveNeighbor);
		if(northBlock->IsValid() &&!belowBlock->IsFullOpaque())
			AddBlockLocatorToDirtyLightingQueue(belowNeighbor);
	}
}

//  =========================================================================================
void World::AddBlockLocatorToDirtyLightingQueue(BlockLocator blockLocator)
{
	if(!blockLocator.IsValid())
		return;

	Block* block = blockLocator.GetBlock();

	//if the block is fully opaque OR it's already in the list, there is no need to add it to the dirty list
	if(block->IsLightingInDirtyList())
		return;

	block->SetLightingInDirtyListFlag(true);

	if(m_isDebugDirtyLighting)
		m_dirtyDebugLightingPoints.emplace_back(blockLocator.m_chunk->GetBlockWorldCenterForBlockIndex(blockLocator.m_blockIndex));

	std::vector<Chunk*> outNeighboringChunks;
	if (blockLocator.IsBlockIndexOnEdge(outNeighboringChunks))
	{
		for (int chunkIndex = 0; chunkIndex < (int)outNeighboringChunks.size(); ++chunkIndex)
		{
			outNeighboringChunks[chunkIndex]->m_isMeshDirty = true;
		}
	}

	m_blocksWithDirtyLighting.emplace_back(blockLocator);
}

//  =========================================================================================
RaycastResult World::Raycast(const Vector3& start, const Vector3& forward, float maxDistance)
{
	// create ray that we are moving along line ----------------------------------------------
	RaycastResult result;
	
	Ray raycast = Ray(start, forward, maxDistance);

	// get chunk information we need for  ----------------------------------------------
	IntVector2 chunkCoordsOfWorldPosition = IntVector2((int)floorf(raycast.m_startPosition.x * BLOCKS_WIDE_X_DIVISOR), (int)floorf(raycast.m_startPosition.y * BLOCKS_WIDE_Y_DIVISOR));

	std::map<IntVector2, Chunk*>::iterator activeChunkIterator = m_activeChunks.find(chunkCoordsOfWorldPosition);

	if (activeChunkIterator == m_activeChunks.end())
	{
		return result;
	}

	Chunk* currentRayChunk = activeChunkIterator->second;

	uint outBlockIndex;
	bool success = currentRayChunk->GetBlockIndexForWorldPositionWithinBounds(outBlockIndex, raycast.m_startPosition);
	
	//we are out of bounds of the chunk bounds
	if (!success)
	{
		return result;
	}
	
	BlockLocator blockLocator = BlockLocator(currentRayChunk, outBlockIndex);
	
	// ready to begin step and sample to find impact ----------------------------------------------
	bool isRaycastComplete = false;
	Vector3 endPosition = raycast.m_startPosition + (forward * maxDistance);	
	Vector3 currentPosition = raycast.m_startPosition;
	IntVector3 currentCoordinates = currentPosition.FloorAndCastToInt();
	float totalStepDistance = 0.f;
	IntVector3 movementDirection = IntVector3::ZERO;
	bool didImpact = false;

	//early out if we started from inside a blcok that is impassable
	if(blockLocator.GetBlock()->IsSolid())
	{
		isRaycastComplete = true;
		didImpact = true;
	}
	
	while (!isRaycastComplete)
	{		
		bool didStepDuringIteration = false;

		totalStepDistance += RAYCAST_STEP_AMOUNT;

		currentPosition += raycast.m_direction * RAYCAST_STEP_AMOUNT;
		IntVector3 newCoordinates = currentPosition.FloorAndCastToInt();
		IntVector3 coordinateDifference = currentCoordinates - newCoordinates;

		if (newCoordinates.x > currentCoordinates.x)
		{
			blockLocator.StepEast();
			movementDirection = IntVector3(1, 0, 0); 
			didStepDuringIteration = true;
			currentCoordinates = newCoordinates;

			if (blockLocator.IsValid())
			{
				if(blockLocator.GetBlock()->IsSolid())
				{
					isRaycastComplete = true;
					didImpact = true;
					break;
				}
			}
			else
			{
				isRaycastComplete = true;
				didImpact = false;
			}
		}
		if (newCoordinates.x < currentCoordinates.x)
		{
			blockLocator.StepWest();
			movementDirection = IntVector3(-1, 0, 0);
			didStepDuringIteration = true;
			currentCoordinates = newCoordinates;

			if (blockLocator.IsValid())
			{
				if(blockLocator.GetBlock()->IsSolid())
				{
					isRaycastComplete = true;
					didImpact = true;
					break;
				}
			}
			else
			{
				isRaycastComplete = true;
				didImpact = false;
			}
		}
		if (newCoordinates.y > currentCoordinates.y)
		{
			blockLocator.StepNorth();
			movementDirection = IntVector3(0, 1, 0);
			didStepDuringIteration = true;
			currentCoordinates = newCoordinates;

			if (blockLocator.IsValid())
			{
				if(blockLocator.GetBlock()->IsSolid())
				{
					isRaycastComplete = true;
					didImpact = true;
					break;
				}
			}
			else
			{
				isRaycastComplete = true;
				didImpact = false;
			}
		}
		if (newCoordinates.y < currentCoordinates.y)
		{
			blockLocator.StepSouth();
			movementDirection = IntVector3(0, -1, 0);
			didStepDuringIteration = true;
			currentCoordinates = newCoordinates;

			if (blockLocator.IsValid())
			{
				if(blockLocator.GetBlock()->IsSolid())
				{
					isRaycastComplete = true;
					didImpact = true;
					break;
				}
			}
			else
			{
				isRaycastComplete = true;
				didImpact = false;
			}
		}
		if (newCoordinates.z > currentCoordinates.z)
		{
			blockLocator.StepUp();
			movementDirection = IntVector3(0, 0, 1);
			didStepDuringIteration = true;
			currentCoordinates = newCoordinates;

			if (blockLocator.IsValid())
			{
				if(blockLocator.GetBlock()->IsSolid())
				{
					isRaycastComplete = true;
					didImpact = true;
					break;
				}
			}
			else
			{
				isRaycastComplete = true;
				didImpact = false;
			}
		}
		if (newCoordinates.z < currentCoordinates.z)
		{
			blockLocator.StepDown();
			movementDirection = IntVector3(0, 0, -1);
			didStepDuringIteration = true;
			currentCoordinates = newCoordinates;

			if (blockLocator.IsValid())
			{
				if(blockLocator.GetBlock()->IsSolid())
				{
					isRaycastComplete = true;
					didImpact = true;
					break;
				}
			}
			else
			{
				isRaycastComplete = true;
				didImpact = false;
			}
		}

		//if we've walked the length of the cast, we didn't hit anything
		if (totalStepDistance >= raycast.m_maxDistance)
		{
			isRaycastComplete = true;
		}

		didStepDuringIteration = false;
	}

	return RaycastResult(raycast, blockLocator, didImpact, currentPosition, endPosition, totalStepDistance, -1 * Vector3(movementDirection));
}

//  =========================================================================================
void World::InitializeSelectableBlockList()
{
	m_selectableBlockTypes.emplace_back(GRASS_BLOCK_ID);
	m_selectableBlockTypes.emplace_back(DIRT_BLOCK_ID);
	m_selectableBlockTypes.emplace_back(STONE_BLOCK_ID);
	m_selectableBlockTypes.emplace_back(GLOWSTONE_BLOCK_ID);
}

//  =========================================================================================
void World::DigBlock()
{
	//early out if we don't have a block to remmove
	if (!m_raycastResult.m_didImpact)
		return;

	BlockLocator impactLocator = m_raycastResult.m_impactBlockLocator;
	if (impactLocator.IsValid())
	{
		SetBlockToType(impactLocator.GetBlock(), AIR_BLOCK_ID);
		AddBlockLocatorToDirtyLightingQueue(impactLocator);
		impactLocator.m_chunk->m_isMeshDirty = true;
		impactLocator.m_chunk->m_doesRequireSave = true;

		BlockLocator aboveBlockLocator = impactLocator.GetBlockLocatorAbove();
		if (aboveBlockLocator.GetBlock()->IsSky())
		{
			AddSkyFlagToBelowBlocks(impactLocator);
		}		
		
		//check to see if we need to set the neighboring chunks to dirty
		std::vector<Chunk*> outNeighboringChunks;
		if (impactLocator.IsBlockIndexOnEdge(outNeighboringChunks))
		{
			for (int chunkIndex = 0; chunkIndex < (int)outNeighboringChunks.size(); ++chunkIndex)
			{
				outNeighboringChunks[chunkIndex]->m_isMeshDirty = true;
			}
		}
	}
}

//  =========================================================================================
void World::PlaceBlock()
{
	//early out if we don't have a place to put the block
	if (!m_raycastResult.m_didImpact)
		return;

	BlockLocator targetedBlockLocator;

	if (m_raycastResult.m_impactNormal == g_north)
	{
		targetedBlockLocator = m_raycastResult.m_impactBlockLocator.GetBlockLocatorToNorth();
	}
	else if (m_raycastResult.m_impactNormal == g_south)
	{
		targetedBlockLocator = m_raycastResult.m_impactBlockLocator.GetBlockLocatorToSouth();
	}
	else if (m_raycastResult.m_impactNormal == g_east)
	{
		targetedBlockLocator = m_raycastResult.m_impactBlockLocator.GetBlockLocatorToEast();
	}
	else if (m_raycastResult.m_impactNormal == g_west)
	{
		targetedBlockLocator = m_raycastResult.m_impactBlockLocator.GetBlockLocatorToWest();
	}
	else if (m_raycastResult.m_impactNormal == g_worldUp)
	{
		targetedBlockLocator = m_raycastResult.m_impactBlockLocator.GetBlockLocatorAbove();
	}
	else if (m_raycastResult.m_impactNormal == -g_worldUp)
	{
		targetedBlockLocator = m_raycastResult.m_impactBlockLocator.GetBlockLocatorBelow();
	}

	if (targetedBlockLocator.IsValid())
	{
		BlockLocator aboveBlockLocator = targetedBlockLocator.GetBlockLocatorAbove();
		if (aboveBlockLocator.GetBlock()->IsSky())
		{
			RemoveSkyFlagFromBelowBlocks(targetedBlockLocator);
		}

		SetBlockToType(targetedBlockLocator.GetBlock(), m_selectableBlockTypes[m_selectedBlockIndex]);
		AddBlockLocatorToDirtyLightingQueue(targetedBlockLocator);
		targetedBlockLocator.m_chunk->m_isMeshDirty = true;
		targetedBlockLocator.m_chunk->m_doesRequireSave = true;

		//check to see if we need to set the neighboring chunks to dirty
		std::vector<Chunk*> outNeighboringChunks;
		if (targetedBlockLocator.IsBlockIndexOnEdge(outNeighboringChunks))
		{
			for (int chunkIndex = 0; chunkIndex < (int)outNeighboringChunks.size(); ++chunkIndex)
			{
				outNeighboringChunks[chunkIndex]->m_isMeshDirty = true;
			}
		}
	}
}

//  =========================================================================================
void World::ToggleCameraViewLocked()
{
	m_isCameraViewLocked = !m_isCameraViewLocked;
}

//  =========================================================================================
void World::CopyCameraDataToPlayerView(const Vector3& cameraPosition, const Vector3& cameraForward)
{
	m_cameraViewPosition = cameraPosition;
	m_cameraViewForwardNormalized = cameraForward;
}

//  =========================================================================================
void World::RemoveSkyFlagFromBelowBlocks(BlockLocator& blockLocator)
{
	//if we replaced dug a block out and we are sky, we need to go down until we hit a non opaque block
	BlockLocator belowBlockLocator = blockLocator.GetBlockLocatorBelow();
	bool isSky = true;
	while (isSky)
	{
		Block* belowBlock = belowBlockLocator.GetBlock();
		if (!belowBlock->IsSky() || !belowBlock->IsValid())
		{
			isSky = false;
		}
		else
		{
			AddBlockLocatorToDirtyLightingQueue(belowBlockLocator);
			belowBlock->SetSkyFlag(false);
			belowBlockLocator.StepDown();
		}
	}	
}

//  =========================================================================================
void World::AddSkyFlagToBelowBlocks(BlockLocator& blockLocator)
{
	Block* targetedBlock = blockLocator.GetBlock();
	targetedBlock->SetSkyFlag(true);

	//if we replaced dug a block out and we are sky, we need to go down until we hit a non opaque block
	BlockLocator belowBlockLocator = blockLocator.GetBlockLocatorBelow();

	bool isSky = true;
	while (isSky)
	{
		Block* belowBlock = belowBlockLocator.GetBlock();
		if (belowBlock->IsFullOpaque() || !belowBlock->IsValid())
		{
			isSky = false;
		}
		else
		{
			AddBlockLocatorToDirtyLightingQueue(belowBlockLocator);
			belowBlock->SetSkyFlag(true);
			//belowBlock->SetOutdoorLightingValue(MAX_OUTDOOR_LIGHTING_VALUE);
			belowBlockLocator.StepDown();
		}
	}	
}

//  =========================================================================================
void World::GetTimeOfDay(float inSeconds, int& outHours, int& outMinutes, int& outSeconds, std::string& outAmPm)
{
	int seconds = (int)inSeconds;

	seconds = seconds % (24 * 3600);
	outSeconds = seconds / 3600;

	seconds %= 3600;
	outMinutes = seconds / 60;

	seconds %= 60;
	outHours = seconds;

	if(inSeconds >= (TIME_PER_DAY_IN_SECONDS * 0.5))
		outAmPm = "PM";
	else
		outAmPm = "AM";
}

//  =========================================================================================
void World::PerlinLightningStrike()
{
	float lightningPerlin = Compute1dPerlinNoise(m_currentTimeOfDay * (float)(m_days + 1), 20.f, 9);

	if(lightningPerlin < 0.9f)
		return;

	float lightningStrength = RangeMapFloat(lightningPerlin, 0.9f, 1.f, 0.f, 1.f);	
	m_globalOutdoorLightColor = Interpolate(m_globalOutdoorLightColor, g_white, lightningStrength);
}

//  =========================================================================================
void World::IndoorLightingFlicker()
{
	float glowPerlin = Compute1dPerlinNoise(m_currentTimeOfDay * (float)(m_days + 1), 500.f, 3);
	float glowStrength = RangeMapFloat(glowPerlin, -1.f, 1.f, 0.8f, 1.f);

	m_globalIndoorLightColor.ScaleRGBByPercentage(glowStrength);
}

//  =========================================================================================
Mesh* World::CreateTexturedUIMesh()
{
	MeshBuilder builder = MeshBuilder();
	Mesh* textMesh = nullptr;
	Window* theWindow = Window::GetInstance();

	// fps counter ----------------------------------------------		
	AABB2 selectedBlockTypeBox = AABB2(theWindow->GetClientWindow(), Vector2(0.95f, 0.9f), Vector2(0.99f, 0.975f));

	BlockDefinition* blockDef = BlockDefinition::GetDefinitionById(m_selectableBlockTypes[m_selectedBlockIndex]);
	AABB2 frontTexCoords = GetTerrainSprites()->GetTexCoordsForSpriteCoords(blockDef->m_frontTexCoords);

	builder.CreateTexturedQuad2D( selectedBlockTypeBox.GetCenter(), selectedBlockTypeBox.GetDimensions(), frontTexCoords.maxs, frontTexCoords.mins, Rgba::WHITE);

	// create mesh ----------------------------------------------
	if (builder.m_vertices.size() > 0)
	{
		textMesh = builder.CreateMesh<VertexPCU>();
	}

	return textMesh;
}

//  =========================================================================================
Mesh* World::CreateUITextMesh()
{
	MeshBuilder builder = MeshBuilder();
	Mesh* textMesh = nullptr;
	Window* theWindow = Window::GetInstance();

	// block type counter ----------------------------------------------		
	AABB2 selectedBlockTypeBox = AABB2(theWindow->GetClientWindow(), Vector2(0.9f, 0.8f), Vector2(0.99f, 0.89f));
	builder.CreateText2DInAABB2( selectedBlockTypeBox.GetCenter(), selectedBlockTypeBox.GetDimensions(), 1.f, Stringf("CurrentBlockType:"), Rgba::WHITE);

	// fps block counter ----------------------------------------------		
	AABB2 fpsBlock = AABB2(theWindow->GetClientWindow(), Vector2(0.9f, 0.75f), Vector2(0.99f, 0.8f));
	builder.CreateText2DFromPoint(fpsBlock.mins, 15.f, 1.f, Stringf("FPS: %-4.2f", m_physicsFPS), Rgba::YELLOW);
	
	//time ----------------------------------------------		
	int seconds = 0;
	int minutes = 0;
	int hours = 0;
	std::string amPm = "";
	GetTimeOfDay(m_currentTimeOfDay, seconds, minutes, hours, amPm);
	AABB2 timeBlock = AABB2(theWindow->GetClientWindow(), Vector2(0.01f, 0.975f), Vector2(0.25f, 0.99f));
	builder.CreateText2DFromPoint(timeBlock.mins, 15.f, 1.f, Stringf("Day: %i - CurrentTime: %i:%01i %s", m_days, hours, minutes, amPm.c_str()), Rgba::YELLOW);

	//camera position ----------------------------------------------		
	AABB2 posBox = AABB2(theWindow->GetClientWindow(), Vector2(0.01f, 0.95f), Vector2(0.25f, 0.975f));
	builder.CreateText2DFromPoint(posBox.mins, 15.f, 1.f, Stringf("CameraPos: %i, %i, %i", (int)m_cameraViewPosition.x, (int)m_cameraViewPosition.y, (int)m_cameraViewPosition.z), Rgba::YELLOW);

	//player ----------------------------------------------		
	AABB2 playerPosBox = AABB2(theWindow->GetClientWindow(), Vector2(0.01f, 0.925f), Vector2(0.25f, 0.95f));
	builder.CreateText2DFromPoint(playerPosBox.mins, 15.f, 1.f, Stringf("PlayerPos: %i, %i, %i", (int)m_player->m_position.x, (int)m_player->m_position.y, (int)m_player->m_position.z), Rgba::YELLOW);

	//player speed  ----------------------------------------------		
	AABB2 playerSpeedBox = AABB2(theWindow->GetClientWindow(), Vector2(0.01f, 0.9f), Vector2(0.25f, 0.925f));
	builder.CreateText2DFromPoint(playerSpeedBox.mins, 15.f, 1.f, Stringf("PlayerSpeed: %f", m_player->GetSpeed()), Rgba::YELLOW);

	//player physics mode  ----------------------------------------------		
	AABB2 physicsModeBox = AABB2(theWindow->GetClientWindow(), Vector2(0.01f, 0.875f), Vector2(0.25f, 0.9f));
	builder.CreateText2DFromPoint(physicsModeBox.mins, 15.f, 1.f, Stringf("Current Mode: %s", m_player->GetPhysicsModeAsText().c_str()), Rgba::YELLOW);

	//debug type - sky mesh ----------------------------------------------		
	if (m_debugSkyMesh != nullptr)
	{
		AABB2 selectedBlockTypeBox = AABB2(theWindow->GetClientWindow(), Vector2(0.01f, 0.8f), Vector2(0.5f, 0.89f));
		builder.CreateText2DInAABB2( selectedBlockTypeBox.GetCenter(), selectedBlockTypeBox.GetDimensions(), 1.f, Stringf("SKY_DEBUG_ENABLED: 'K' to Refresh & '1' to disable"), Rgba::YELLOW);
	}

	//debug type - dirty lighting stepping ----------------------------------------------		
	if (m_isDebugDirtyLighting)
	{
		AABB2 selectedBlockTypeBox = AABB2(theWindow->GetClientWindow(), Vector2(0.01, 0.7f), Vector2(0.5f, 0.79f));
		builder.CreateText2DInAABB2( selectedBlockTypeBox.GetCenter(), selectedBlockTypeBox.GetDimensions(), 1.f, Stringf("DEBUG_DIRTY_LIGHTING ENABLED: 'L' to Step & '2' to disable"), Rgba::YELLOW);
	}

	// create mesh ----------------------------------------------
	if (builder.m_vertices.size() > 0)
	{
		textMesh = builder.CreateMesh<VertexPCU>();
	}

	return textMesh;
}

//  =========================================================================================
void World::GenerateDebugSkyMesh()
{
	Vector3 position = m_gameCamera->m_position;
	Chunk* chunk = GetActiveChunkFromPlayerPosition(position);

	if(chunk == nullptr)
		return;

	MeshBuilder debugSkyBuilder;

	for (int columnIndex = 0; columnIndex < BLOCKS_PER_LAYER; ++columnIndex)
	{
		bool isBlockSky = true;
		//get highest block in column
		IntVector3 blockCoords = chunk->GetBlockCoordsForBlockIndex(columnIndex);
		blockCoords.z = BLOCKS_HIGH_Z - 1; //change index to the highest in the column

		int blockIndex = chunk->GetBlockIndexForBlockCoords(blockCoords);
		BlockLocator currentBlockLocator = BlockLocator(chunk, blockIndex);

		while (isBlockSky)
		{
			//get the block locator below
			Block* blockToProcess = currentBlockLocator.GetBlock();

			//if block is visible we are at the 'ground' (first non air/sky block) at the current index
			if (!blockToProcess->IsSky())
			{
				isBlockSky = false;
			}
			else
			{			
				//set block locator to block locator below
				Vector3 worldCenterOfBlock = chunk->GetBlockWorldCenterForBlockIndex(currentBlockLocator.m_blockIndex);

				CreateDebugStar(debugSkyBuilder, worldCenterOfBlock, Rgba::PINK, 0.25f);
				currentBlockLocator.StepDown();
			}
		}
	}

	m_debugSkyMesh = debugSkyBuilder.CreateMesh<VertexPCU>();
}

//  =========================================================================================
Chunk* World::GetActiveChunkFromPlayerPosition(const Vector3& playerPosition)
{
	std::map<IntVector2, Chunk*>::iterator activeChunkIterator = m_activeChunks.begin();

	for (activeChunkIterator; activeChunkIterator != m_activeChunks.end(); ++activeChunkIterator)
	{
		Chunk* chunk = activeChunkIterator->second;
		if(chunk->m_worldBounds.IsPointInside(playerPosition))
			return chunk;
	}

	//point not found within active chunk list
	return nullptr;
}

//  =========================================================================================
bool CompareDistanceFromZeroLessThan(const IntVector2& first, const IntVector2& second)
{
	int firstDistanceFromZero = GetDistanceSquared(IntVector2::ZERO, first);
	int secondDistanceFromZero = GetDistanceSquared(IntVector2::ZERO, second);

	if(firstDistanceFromZero < secondDistanceFromZero)
		return true;

	return false;
}

//  =========================================================================================
bool CompareDistanceFromLocationToLocationLessThanRadius(float& outDistanceSquared, const Vector2& startLocation, const Vector2& endLocation, const float radius)
{
	outDistanceSquared = GetDistanceSquared(startLocation, endLocation);
	float radiusSquared = radius * radius;

	if(outDistanceSquared < radiusSquared)
		return true;

	return false;

}

//  =========================================================================================
bool CompareDistanceFromLocationToLocationGreaterThanRadius(float& outDistanceSquared, const Vector2& startLocation, const Vector2& endLocation, const float radius)
{
	outDistanceSquared = GetDistanceSquared(startLocation, endLocation);
	float radiusSquared = radius * radius;

	if(outDistanceSquared > radiusSquared)
		return true;

	return false;

}

