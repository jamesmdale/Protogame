#pragma once
#include <vector>
#include "Engine\Time\Clock.hpp"
#include "Engine\Camera\Camera.hpp"
#include "Game\GameCommon.hpp"

class NetMessage;
class NetConnection;
enum eNetMessageFlag;
class Spritesheet;
class Stopwatch;

class Game
{
public:  
	//camera members
	Camera * m_engineCamera = nullptr;
	Camera* m_uiCamera = nullptr;

	//clock
	Clock* m_gameClock = nullptr;

	SpriteSheet* m_terrainSprites = nullptr;
	Stopwatch* m_inputDelayTimer = nullptr;

public:
	Game();
	~Game();
	static Game* GetInstance();
	static Game* CreateInstance();

	void Initialize();

	void Update(); //use internal clock for delta seconds;
	void PreRender();
	void Render();
	void PostRender();

	void RegisterGameNetMessages();

	float UpdateInput(float deltaSeconds);
};

SpriteSheet* GetTerrainSprites();







