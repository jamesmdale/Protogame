#pragma once
#include "Engine\Time\Stopwatch.hpp"
#include "Engine\Core\Widget.hpp"
#include "Game\GameObjects\Player.hpp"
#include "Game\GameStates\GameState.hpp"
#include "Game\GameObjects\GameCamera.hpp"
#include "Game\GameObjects\World.hpp"

class Player;

class PlayingState : public GameState
{
public:
	PlayingState(Camera* camera) : GameState(camera)
	{
		m_type = PLAYING_GAME_STATE;
	}

	virtual ~PlayingState() override;
	
	virtual void Initialize() override;
	virtual void Update(float deltaSeconds) override;
	virtual void PreRender() override;
	virtual void Render() override;
	virtual void PostRender() override;
	virtual float UpdateFromInput(float deltaSeconds) override;

	float UpdateFromInputDebug(float deltaSeconds);

public:

	World* m_world = nullptr;
};

