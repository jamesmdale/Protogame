#include "Game\GameObjects\Player.hpp"
#include "Game\Game.hpp"
#include "Game\GameCommon.hpp"
#include "Game\Helpers\GameRendererHelpers.hpp"
#include "Game\GameObjects\Block.hpp"
#include "Game\GameObjects\Chunk.hpp"
#include "Game\GameObjects\World.hpp"
#include "Engine\TIme\Stopwatch.hpp"
#include "Engine\Renderer\Mesh.hpp"
#include "Engine\Input\InputSystem.hpp"
#include "Engine\Core\EngineCommon.hpp"

//  =========================================================================================
Player::Player(World* world) : Entity(world)
{
	//generate visual bounds
	Vector3 bottomPivot = Vector3(0.f, 0.f, 0.f);
	float widthRadius = g_playerWidth / 2.f;
	float heightRadius = g_playerHeight / 2.f;
	m_visualBounds = AABB3(Vector3::ZERO, widthRadius, widthRadius, heightRadius);

	//generate physics sphere bounds
	m_physicsSphere = Sphere(Vector3::ZERO, widthRadius);

	m_firstPersonCameraPositionOffsetFromPivot = Vector3(0.f, 0.f, g_playerEyesOffset);
	m_thirdPersonDistanceOffset = 4.f; //4 meters

	if (m_visualsMesh == nullptr)
		GenerateDebugVisualsMesh();
	if (m_physicsMesh == nullptr)
		GenerateDebugPhysicsMesh();

	m_doesReceiveUserInput = true;
}

//  =========================================================================================
Player::~Player()
{
	//nothing to delete yet
}

//  =========================================================================================
void Player::Update(float deltaSeconds)
{
	UpdateIsOnGround();
}

//  =========================================================================================
void Player::UpdatePhysics(float deltaSeconds)
{
	//no clip logic skips corrective physics and speed limits (doesn't even use velocity)
	if (m_currentPhysicsMode == NO_CLIP_PHYSICS_MODE)
	{
		m_velocity += m_frameMoveIntention * g_playerWalkSpeed * deltaSeconds;
		Vector3 moveDirection = m_velocity.GetNormalized();
		m_velocity = Vector3::ZERO;
		m_position += moveDirection * g_playerWalkSpeed * deltaSeconds;
		m_frameMoveIntention = Vector3::ZERO;
		UpdateBoundsToCurrentPosition();
		return;
	}
		
	//apply neutonian physics
	float speedBeforeChange = m_velocity.GetLengthXY();
	Vector3 accelerationXY = Vector3(m_frameMoveIntention.x, m_frameMoveIntention.y, 0.f) * g_playerWalkAcceleration;//* deltaSeconds;
	Vector3 moveIntentionZ = Vector3(0.f, 0.f, m_frameMoveIntention.z);

	m_velocity += (accelerationXY + moveIntentionZ);

	//apply all forces into velocity
	Vector3 gravity = g_gravity;
	if (m_currentPhysicsMode == FLYING_PHYSICS_MODE)
		gravity = Vector3::ZERO;

	m_velocity += (gravity * deltaSeconds);

	//apply friction
	ApplyFrictionToVelocity(deltaSeconds);

	float speedAfterChange = m_velocity.GetLengthXY();

	//apply speed limits
	if (speedAfterChange > speedBeforeChange)
	{
		float speedLimit = g_playerWalkSpeed;
		if (m_currentPhysicsMode == FLYING_PHYSICS_MODE)
			speedLimit = g_playerFlySpeed;

		float maxSpeedLimit = GetMaxFloat(speedBeforeChange, speedLimit);
		m_velocity.ClipXYToLength(maxSpeedLimit);
	}

	//move the player
	m_position += m_velocity * deltaSeconds;
	UpdateBoundsToCurrentPosition();

	//corrective physics
	std::vector<BlockLocator> neighborhood;
	GetBlockNeighborhood(neighborhood);

	int pushCount = 0;
	for (int blockIndex = 0; blockIndex < (int)neighborhood.size() && pushCount < 3; ++blockIndex)
	{
		bool didPush = PushOutOfBlock(neighborhood[blockIndex]);
		if (didPush)
			pushCount += 1;
	}

	//update frame move intention
	m_frameMoveIntention = Vector3::ZERO;
}

//  =========================================================================================
void Player::Render()
{
	//if the camera is attached to us and is in first person, don't draw anything
	if (m_attachedCamera != nullptr)
		if (m_attachedCamera->m_currentCameraMode == FIRST_PERSON_CAMERA_MODE)
			return;

	Renderer* theRenderer = Renderer::GetInstance();
	theRenderer->BindMaterial(theRenderer->CreateOrGetMaterial("default"));
	theRenderer->SetTexture(*theRenderer->CreateOrGetTexture("default"));

	if (m_visualsMesh == nullptr)
		GenerateDebugVisualsMesh();
	if (m_physicsMesh == nullptr)
		GenerateDebugPhysicsMesh();

	Matrix44 meshMatrix;

	//position debug draw
	Sphere sphere = Sphere(m_position, 0.25f);
	Mesh* debugPositionMesh = CreateGameUVSphere(sphere, Rgba::PINK, 0.05f);
	theRenderer->DrawMesh(debugPositionMesh);

	//draw visual bounds
	meshMatrix.SetTranslation(m_visualBounds.GetCenter());
	theRenderer->DrawMesh(m_visualsMesh, meshMatrix);

	Vector3 pivotWorldPosition = GetBottomCenterPivot();

	Mesh* debugPivotMesh = CreateDebugStar(pivotWorldPosition, Rgba::PINK, 0.05f);
	theRenderer->DrawMesh(debugPivotMesh);

	//draw physics sphere
	Matrix44 physicsMatrix;
	Vector3 physicsSpherePosition = Vector3(pivotWorldPosition.x, pivotWorldPosition.y, pivotWorldPosition.z + m_physicsSphere.m_radius);
	physicsMatrix.SetTranslation(physicsSpherePosition);
	theRenderer->DrawMesh(m_physicsMesh, physicsMatrix);

	Mesh* debugVelocityMesh = CreateLine(pivotWorldPosition, pivotWorldPosition + m_velocity, Rgba::YELLOW);
	theRenderer->DrawMesh(debugVelocityMesh);

	delete(debugPivotMesh);
	delete(debugPositionMesh);
}

//  =========================================================================================
void Player::PreRender()
{
	//move camera to it's position
	if (m_attachedCamera == nullptr)
		return;

	switch (m_attachedCamera->m_currentCameraMode)
	{		
		case FIRST_PERSON_CAMERA_MODE:
			SetFirstPersonCamera();
			break;
		case THIRD_PERSON_CAMERA_MODE:
			SetThirdPersonCamera();
			break;
			/*case FIXED_ANGLE_CAMERA_MODE:
				break;*/
		case DETACHED_CAMERA_MODE:
			break;
		case NUM_CAMERA_MODES:
			break;		
	}
}

//  =========================================================================================
float Player::UpdateFromInput(float deltaSeconds)
{
	//early out if input is disabled for entity
	if (!DoesReceiveInput())
		return deltaSeconds;

	InputSystem* theInput = InputSystem::GetInstance();
	Game* theGame = Game::GetInstance();

	//handle rotation
	Vector2 mouseDelta = Vector2::ZERO;
	mouseDelta = InputSystem::GetInstance()->GetMouse()->GetMouseDelta();

	m_yawDegreesZ += -mouseDelta.x * 0.05f;
	m_pitchDegreesY += mouseDelta.y * 0.05f;

	m_yawDegreesZ = Modulus(m_yawDegreesZ, 360.f);
	m_pitchDegreesY = ClampFloat(m_pitchDegreesY, -90.f, 90.f);

	Vector3 playerForward = Vector3(CosDegrees(m_yawDegreesZ), SinDegrees(m_yawDegreesZ), 0);
	Vector3 playerRight = Vector3(SinDegrees(m_yawDegreesZ), -CosDegrees(m_yawDegreesZ), 0); //could make cheaper

	//clear move intention before applying from input
	m_frameMoveIntention = Vector3::ZERO;

	//forward
	if (theInput->IsKeyPressed(theInput->KEYBOARD_W))
	{
		//calculate movement for camera and use same movement for ship and light
		m_frameMoveIntention += playerForward;
	}

	//backward
	if (theInput->IsKeyPressed(theInput->KEYBOARD_S))
	{
		m_frameMoveIntention += -playerForward;
	}

	//left
	if (theInput->IsKeyPressed(theInput->KEYBOARD_A))
	{
		m_frameMoveIntention += -playerRight;
	}

	//right
	if (theInput->IsKeyPressed(theInput->KEYBOARD_D))
	{
		m_frameMoveIntention += playerRight;
	}

	m_frameMoveIntention.Normalize();

	//jump (World up)
	if (theInput->WasKeyJustPressed(theInput->KEYBOARD_SPACE) && theGame->m_inputDelayTimer->HasElapsed())
	{
		TODO("separate jump into bool isJumping. For cases like flying etc");
		if(m_isOnGround)
			m_frameMoveIntention.z = g_jumpStrength;

		theGame->m_inputDelayTimer->Reset();
	}

	return deltaSeconds;
}

//  =========================================================================================
void Player::UpdateIsOnGround()
{
	//get a position just slightly below the current position
	Vector3 bottomCenterPivot = GetBottomCenterPivot();
	bottomCenterPivot -= Vector3(0.f, 0.f, 0.01f);

	//get the block for that value
	BlockLocator currentBlock = m_world->GetChunkByPositionFromChunkList(bottomCenterPivot);

	currentBlock.GetBlock()->IsSolid() ? m_isOnGround = true : m_isOnGround = false;
}	

//  =========================================================================================
void Player::SetCamera(GameCamera* camera)
{
	m_attachedCamera = camera;
}

//  =========================================================================================
void Player::GenerateDebugVisualsMesh()
{	
	m_visualsMesh = CreateCubeOutline(m_visualBounds, Rgba::LIGHT_PINK_TRANSPARENT);
}

//  =========================================================================================
void Player::GenerateDebugPhysicsMesh()
{
	m_physicsMesh = CreateGameUVSphere(m_physicsSphere, Rgba::LIGHT_BLUE_TRANSPARENT);
}

//  =========================================================================================
Vector3 Player::GetBottomCenterPivot()
{
	Vector3 boundsCenter = m_visualBounds.GetCenter();
	return Vector3(boundsCenter.x, boundsCenter.y, boundsCenter.z - (m_visualBounds.GetDimensions().z / 2.f));
}

//  =========================================================================================
void Player::UpdateBoundsToCurrentPosition()
{
	m_visualBounds.SetCenter(m_position);

	Vector3 bottomCenterPivot = GetBottomCenterPivot();
	m_physicsSphere.m_position = Vector3(bottomCenterPivot.x, bottomCenterPivot.y, bottomCenterPivot.z + m_physicsSphere.m_radius);
}

//  =========================================================================================
void Player::SetFirstPersonCamera()
{
	if (m_attachedCamera != nullptr)
		if (m_attachedCamera->m_currentCameraMode == FIRST_PERSON_CAMERA_MODE)
		{
			Vector3 eyePos = GetBottomCenterPivot() + m_firstPersonCameraPositionOffsetFromPivot;
			m_attachedCamera->SetTranslation(eyePos);
			m_attachedCamera->SetRotations(m_rollDegreesX, m_pitchDegreesY, m_yawDegreesZ);
		}		
}

//  =========================================================================================
void Player::SetThirdPersonCamera()
{
	if (m_attachedCamera != nullptr)
	{
		if (m_attachedCamera->m_currentCameraMode == THIRD_PERSON_CAMERA_MODE)
		{
			Vector3 eyePos = GetBottomCenterPivot() + m_firstPersonCameraPositionOffsetFromPivot;
			Vector3 forward = GetForward();
			m_attachedCamera->SetTranslation(eyePos + (-1.f * forward * 4.f));
			m_attachedCamera->SetRotations(m_rollDegreesX, m_pitchDegreesY, m_yawDegreesZ);
		}
	}
}

//  =========================================================================================
void Player::ApplyFrictionToVelocity(float deltaSeconds)
{
	Vector3 velocityNormalized = m_velocity.GetNormalized();
	if (m_isOnGround) 
	{
		//we are on ground. Apply ground friction
		Vector3 frictionForce = Vector3(m_velocity.x * g_groundFrictionAmount, m_velocity.y * g_groundFrictionAmount, 0.f);
		m_velocity += (-1.f * frictionForce * deltaSeconds);
	}
	else
	{
		//we are in air. Apply air friction
		float airFrictionZ = (m_currentPhysicsMode == FLYING_PHYSICS_MODE) ? g_airFrictionAmount : 0.f;
		Vector3 frictionForce = Vector3(m_velocity.x * g_airFrictionAmount, m_velocity.y * g_airFrictionAmount, m_velocity.z * airFrictionZ);
		m_velocity += (-1.f * frictionForce * deltaSeconds);
	}
}

//  =========================================================================================
void Player::GetBlockNeighborhood(std::vector<BlockLocator>& outBlockLocators)
{
	if (m_world == nullptr)
		return;

	BlockLocator currentBlock = m_world->GetChunkByPositionFromChunkList(m_physicsSphere.m_position);

	if (!currentBlock.IsValid())
		return;

	//top priority
	BlockLocator checkedBlock = currentBlock.GetBlockLocatorBelow();
	if(checkedBlock.IsValid())
		outBlockLocators.push_back(checkedBlock);

	checkedBlock = currentBlock.GetBlockLocatorAbove();
	if (checkedBlock.IsValid())
	outBlockLocators.push_back(checkedBlock);

	checkedBlock = currentBlock.GetBlockLocatorToNorth();
	if (checkedBlock.IsValid())
	outBlockLocators.push_back(checkedBlock);

	checkedBlock = currentBlock.GetBlockLocatorToSouth();
	if (checkedBlock.IsValid())
	outBlockLocators.push_back(checkedBlock);

	checkedBlock = currentBlock.GetBlockLocatorToEast();
	if (checkedBlock.IsValid())
	outBlockLocators.push_back(checkedBlock);

	checkedBlock = currentBlock.GetBlockLocatorToWest();
	if (checkedBlock.IsValid())
	outBlockLocators.push_back(checkedBlock);

	//second priority
	checkedBlock = currentBlock.GetBlockLocatorToNorthEast();
	if (checkedBlock.IsValid())
	outBlockLocators.push_back(checkedBlock);

	checkedBlock = currentBlock.GetBlockLocatorToNorthWest();
	if (checkedBlock.IsValid())
	outBlockLocators.push_back(checkedBlock);

	checkedBlock = currentBlock.GetBlockLocatorToSouthEast();
	if (checkedBlock.IsValid())
	outBlockLocators.push_back(checkedBlock);

	checkedBlock = currentBlock.GetBlockLocatorToSouthWest();
	if (checkedBlock.IsValid())
	outBlockLocators.push_back(checkedBlock);

	checkedBlock = currentBlock.GetBlockLocatorToBelowNorth();
	if (checkedBlock.IsValid())
	outBlockLocators.push_back(checkedBlock);

	checkedBlock = currentBlock.GetBlockLocatorToBelowSouth();
	if (checkedBlock.IsValid())
	outBlockLocators.push_back(checkedBlock);

	checkedBlock = currentBlock.GetBlockLocatorToBelowEast();
	if (checkedBlock.IsValid())
	outBlockLocators.push_back(checkedBlock);

	checkedBlock = currentBlock.GetBlockLocatorToBelowWest();
	if (checkedBlock.IsValid())
	outBlockLocators.push_back(checkedBlock);

	checkedBlock = currentBlock.GetBlockLocatorToAboveNorth();
	if (checkedBlock.IsValid())
	outBlockLocators.push_back(checkedBlock);

	checkedBlock = currentBlock.GetBlockLocatorToAboveSouth();
	if (checkedBlock.IsValid())
	outBlockLocators.push_back(checkedBlock);

	checkedBlock = currentBlock.GetBlockLocatorToAboveEast();
	if (checkedBlock.IsValid())
	outBlockLocators.push_back(checkedBlock);

	checkedBlock = currentBlock.GetBlockLocatorToAboveWest();
	if (checkedBlock.IsValid())
	outBlockLocators.push_back(checkedBlock);

	//third priority
	checkedBlock = currentBlock.GetBlockLocatorToBelowSouthEast();
	if (checkedBlock.IsValid())
	outBlockLocators.push_back(checkedBlock);

	checkedBlock = currentBlock.GetBlockLocatorToBelowSouthWest();
	if (checkedBlock.IsValid())
	outBlockLocators.push_back(checkedBlock);

	checkedBlock = currentBlock.GetBlockLocatorToBelowNorthEast();
	if (checkedBlock.IsValid())
	outBlockLocators.push_back(checkedBlock);

	checkedBlock = currentBlock.GetBlockLocatorToBelowNorthWest();
	if (checkedBlock.IsValid())
	outBlockLocators.push_back(checkedBlock);

	checkedBlock = currentBlock.GetBlockLocatorToAboveNorthEast();
	if (checkedBlock.IsValid())
	outBlockLocators.push_back(checkedBlock);

	checkedBlock = currentBlock.GetBlockLocatorToAboveNorthWest();
	if (checkedBlock.IsValid())
	outBlockLocators.push_back(checkedBlock);

	checkedBlock = currentBlock.GetBlockLocatorToAboveSouthEast();
	if (checkedBlock.IsValid())
	outBlockLocators.push_back(checkedBlock);

	checkedBlock = currentBlock.GetBlockLocatorToAboveSouthWest();
	if (checkedBlock.IsValid())
	outBlockLocators.push_back(checkedBlock);
}

//  =========================================================================================
bool Player::PushOutOfBlock(BlockLocator locator)
{
	if (!locator.IsValid())
		return false;

	Block* block = locator.GetBlock();
	if (!locator.GetBlock()->IsSolid())
		return false;

	Vector3 worldCenter = locator.m_chunk->GetBlockWorldCenterForBlockIndex(locator.m_blockIndex);
	AABB3 blockBounds = AABB3(worldCenter, 0.5f, 0.5f, 0.5f);
	
	//determine if point is in 
	Vector3 closestPointOnBounds = blockBounds.GetClosestPointOnAABB3(m_physicsSphere.m_position);

	//we aren't overlapping so no need to push out
	float distanceFromSphereToClosestPointOnAABB3 = GetDistance(closestPointOnBounds, m_physicsSphere.m_position);
	float separation = distanceFromSphereToClosestPointOnAABB3 - m_physicsSphere.m_radius;

	//we aren't overlapping
	if (separation >= 0.f || IsNearZero(separation))
		return false;
	
	//we need to push out
	Vector3 pushDirectionNormalized = (m_physicsSphere.m_position - closestPointOnBounds).GetNormalized();		
	m_position += (pushDirectionNormalized * (-1.f * separation));

	UpdateBoundsToCurrentPosition();

	//kill velocity in the direction we are pushing
	Vector3 projection = GetProjectedVector(m_velocity, (-1.f * pushDirectionNormalized));
	m_velocity -= projection;

	return true;
}
