#include <stdlib.h>
#include <vector>
#include <string>
#include "Engine\Math\MathUtils.hpp"
#include "Engine\Renderer\Renderer.hpp"
#include "Engine\Math\Matrix44.hpp"
#include "Engine\Core\EngineCommon.hpp"
#include "Engine\Math\Vector3.hpp"
#include "Engine\Core\Command.hpp"
#include "Engine\Core\DevConsole.hpp"
#include "Engine\Window\Window.hpp"
#include "Engine\Renderer\MeshBuilder.hpp"
#include "Engine\Camera\OrbitCamera.hpp"
#include "Engine\Core\Widget.hpp"
#include "Engine\Debug\DebugRender.hpp"
#include "Engine\Core\StringUtils.hpp"
#include "Engine\Core\EngineCommon.hpp"
#include "Engine\Core\DevConsole.hpp"
#include "Engine\Renderer\SpriteSheet.hpp"
#include "Engine\Time\Stopwatch.hpp"
#include "Game\Game.hpp"
#include "Game\GameCommon.hpp"
#include "Game\GameStates\GameState.hpp"
#include "Game\GameStates\MainMenuState.hpp"
#include "Game\GameStates\LoadingState.hpp"
#include "Game\GameStates\PlayingState.hpp"
#include "Game\Definitions\BlockDefinition.hpp"


//game instance
static Game* g_theGame = nullptr;
static SpriteSheet* g_terrainSprites = nullptr;

bool m_isPaused = false;

//  =========================================================================================
Game::Game()
{

}

//  =========================================================================================
Game::~Game()
{
	//delete camera members
	delete(m_engineCamera);
	m_engineCamera = nullptr;

	delete(m_uiCamera);
	m_uiCamera = nullptr;

	//cleanup global members

	//add any other data to cleanup
}

//  =========================================================================================
Game* Game::GetInstance()
{
	return g_theGame;
}

//  =========================================================================================
Game* Game::CreateInstance()
{
	if (g_theGame == nullptr)
	{
		g_theGame = new Game();
	}

	return g_theGame;
}

//  =========================================================================================
void Game::Initialize()
{
	Window* theWindow = Window::GetInstance();
	Renderer* theRenderer = Renderer::GetInstance();

	theRenderer->SetAmbientLightIntensity(0.15f);

	m_gameClock = new Clock(GetMasterClock());

	// add game cameras
	m_engineCamera = new Camera();
	m_engineCamera->SetDepthStencilTarget(theRenderer->GetDefaultDepthStencilTarget());
	m_engineCamera->SetColorTarget(theRenderer->GetDefaultRenderTarget());
	m_engineCamera->SetPerspective(60.f, CLIENT_ASPECT, 0.1f, 10000.f);

	//m_gameCamera->m_skybox = new Skybox("Data/Images/galaxy2.png");
	//add ui camera
	m_uiCamera = new Camera();
	m_uiCamera->SetColorTarget(theRenderer->GetDefaultRenderTarget());
	m_uiCamera->SetOrtho(0.f, theWindow->m_clientWidth, 0.f, theWindow->m_clientHeight, -1.f, 1.f);
	m_uiCamera->SetView(Matrix44::IDENTITY);

	// add menu states
	TODO("Add other menu states");
	GameState::AddGameState(new MainMenuState(m_uiCamera));
	GameState::AddGameState(new PlayingState(m_engineCamera));

	// set to initial menu
	GameState::TransitionGameStatesImmediate(GameState::GetGameStateFromGlobalListByType(MAIN_MENU_GAME_STATE));
	GameState::UpdateGlobalGameState(0.f);

	//initialize definitions
	BlockDefinition::Initialize("Data/Definitions/BlockDefinitions.xml");
	g_terrainSprites = new SpriteSheet(*theRenderer->CreateOrGetTexture("Data/Images/Terrain_32x32.png"), 32, 32);

	m_inputDelayTimer = new Stopwatch(GetMasterClock());
	m_inputDelayTimer->SetTimer(PLAYER_INPUT_DELAY_IN_SECONDS);

	// cleanup
	theRenderer = nullptr;
	theWindow = nullptr;
}

//  =========================================================================================
void Game::Update()
{
	float deltaSeconds = m_gameClock->GetDeltaSeconds();

	// update global menu data (handles transitions and timers)
	GameState::UpdateGlobalGameState(deltaSeconds);

	GameState::GetCurrentGameState()->Update(deltaSeconds);
}


//  =========================================================================================
void Game::PreRender()
{
	GameState::GetCurrentGameState()->PreRender();
}

//  =========================================================================================
void Game::Render()
{
	GameState::GetCurrentGameState()->Render();
}

//  =========================================================================================
void Game::PostRender()
{
	GameState::GetCurrentGameState()->PostRender();
}

//  =============================================================================
void Game::RegisterGameNetMessages()
{

}

//  =========================================================================================
float Game::UpdateInput(float deltaSeconds)
{
	deltaSeconds = GameState::GetCurrentGameState()->UpdateFromInput(deltaSeconds);

	return deltaSeconds;
}

//  =========================================================================================
SpriteSheet* GetTerrainSprites()
{
	return g_terrainSprites;
}
