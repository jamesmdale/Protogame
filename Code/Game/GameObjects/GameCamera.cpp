#include "Game\GameObjects\GameCamera.hpp"
#include "Game\GameObjects\Entity.hpp"
#include "Game\GameCommon.hpp"
#include "Game\Game.hpp"
#include "Game\GameStates\GameState.hpp"
#include "Game\GameStates\PlayingState.hpp"
#include "Engine\Input\InputSystem.hpp"



//  =========================================================================================
GameCamera::GameCamera()
{
	m_currentCameraMode = FIRST_PERSON_CAMERA_MODE;
}

//  =========================================================================================
GameCamera::~GameCamera()
{
}

//  =========================================================================================
void GameCamera::UpdateFromInput(float deltaSeconds)
{
	if (m_currentCameraMode != DETACHED_CAMERA_MODE)
		return;

	InputSystem* theInput = InputSystem::GetInstance();
	Game* theGame = Game::GetInstance();

	Vector2 mouseDelta = Vector2::ZERO;
	mouseDelta = InputSystem::GetInstance()->GetMouse()->GetMouseDelta();

	//calculate rotation for camera and use same rotation for player
	//m_camera->m_transform->AddRotation(Vector3(mouseDelta.y, mouseDelta.x, 0.f) * deltaSeconds * 10.f);

	m_yawDegreesZ += -mouseDelta.x * 0.05f;
	m_pitchDegreesY += mouseDelta.y * 0.05f;

	m_yawDegreesZ = Modulus(m_yawDegreesZ, 360.f);
	m_pitchDegreesY = ClampFloat(m_pitchDegreesY, -90.f, 90.f);

	Vector3 cameraCardinalForward = Vector3(CosDegrees(m_yawDegreesZ), SinDegrees(m_yawDegreesZ), 0);
	cameraCardinalForward.Normalize();
	Vector3 cameraRight = Vector3(SinDegrees(m_yawDegreesZ), -CosDegrees(m_yawDegreesZ), 0);

	Vector3 positionToAdd = Vector3::ZERO;
	Vector3 positionAtStartOfFrame = positionToAdd;

	//update movement
	//forward (x)
	if (theInput->IsKeyPressed(theInput->KEYBOARD_W))
	{
		//calculate movement for camera and use same movement for ship and light
		positionToAdd = cameraCardinalForward * deltaSeconds * CAMERA_FLY_SPEED;
	}

	//backward (-x)
	if (theInput->IsKeyPressed(theInput->KEYBOARD_S))
	{
		positionToAdd = -cameraCardinalForward * deltaSeconds * CAMERA_FLY_SPEED;
	}

	//left is north (y)
	if (theInput->IsKeyPressed(theInput->KEYBOARD_A))
	{
		positionToAdd = -cameraRight * deltaSeconds * CAMERA_FLY_SPEED;
	}

	//right is south (-y)
	if (theInput->IsKeyPressed(theInput->KEYBOARD_D))
	{
		positionToAdd = cameraRight * deltaSeconds * CAMERA_FLY_SPEED;
	}

	//up is (+z)
	if (theInput->IsKeyPressed(theInput->KEYBOARD_SPACE) || theInput->IsKeyPressed(theInput->KEYBOARD_E))
	{
		positionToAdd = g_worldUp * deltaSeconds * CAMERA_FLY_SPEED;
	}

	//up is (-z)
	if (theInput->IsKeyPressed(theInput->KEYBOARD_CONTROL) || theInput->IsKeyPressed(theInput->KEYBOARD_Q))
	{
		positionToAdd = -g_worldUp * deltaSeconds * CAMERA_FLY_SPEED;
	}

	//sprint move speed (twice as fast)
	if (theInput->IsKeyPressed(theInput->KEYBOARD_SHIFT))
	{
		positionToAdd *= 3.f;
	}

	//walk move speed
	if (theInput->IsKeyPressed(theInput->KEYBOARD_C))
	{
		positionToAdd *= 0.15f;
	}

	Translate(positionToAdd);
}

//  =========================================================================================
void GameCamera::CreateFliippedViewMatrix(Matrix44& outMatrix)
{
	outMatrix.SetIdentity();
	outMatrix.Append(g_flipMatrix);
	outMatrix.RotateDegreesAroundX3D(-1.f * m_rollDegreesX);
	outMatrix.RotateDegreesAroundY3D(-1.f * m_pitchDegreesY);
	outMatrix.RotateDegreesAroundZ3D(-1.f * m_yawDegreesZ);
	outMatrix.Translate3D(-1.f * m_position);
}

//  =========================================================================================
void GameCamera::Translate(const Vector3& translation)
{
	m_position += translation;
}

//  =========================================================================================
void GameCamera::AttachToEntity(Entity* entity)
{
	m_attachedEntity = entity;
	entity->SetCamera(this);

	//enable disable inputs depending on the current mode
	switch (m_currentCameraMode)
	{
		case FIRST_PERSON_CAMERA_MODE:
			m_attachedEntity->EnableInput();
			break;
			case THIRD_PERSON_CAMERA_MODE:
				m_attachedEntity->EnableInput();
				break;
			/*case FIXED_ANGLE_CAMERA_MODE:
				m_attachedEntity->EnableInput();
				break;*/
			/*case STATIONARY_CAMERA_MODE:
				m_attachedEntity->m_attachedCamera = nullptr;
				m_attachedEntity = nullptr;
				break;*/
			
		case DETACHED_CAMERA_MODE:
			m_attachedEntity->DisableInput();
			break;
		case NUM_CAMERA_MODES:
			break;

	}
}

//  =========================================================================================
void GameCamera::SetRotations(float rollX, float pitchY, float yawZ)
{
	m_rollDegreesX = rollX;
	m_pitchDegreesY = pitchY;
	m_yawDegreesZ = yawZ;
}

//  =========================================================================================
void GameCamera::SetTranslation(const Vector3& position)
{
	m_position = position;
}

//  =========================================================================================
void GameCamera::CycleCameraModes()
{
	PlayingState* gameState = (PlayingState*)GameState::GetCurrentGameState();
	if (gameState->m_type != PLAYING_GAME_STATE)
		return;

	m_currentCameraMode = (CameraModes)((int)m_currentCameraMode + 1);

	if (m_currentCameraMode == NUM_CAMERA_MODES)
		m_currentCameraMode = (CameraModes)0;

	switch (m_currentCameraMode)
	{
	case FIRST_PERSON_CAMERA_MODE:
		gameState->m_world->m_player->SetCamera(this);
		m_attachedEntity = gameState->m_world->m_player;
		m_attachedEntity->EnableInput();
		break;
	case THIRD_PERSON_CAMERA_MODE:
		gameState->m_world->m_player->SetCamera(this);
		m_attachedEntity = gameState->m_world->m_player;
		m_attachedEntity->EnableInput();
		break;
	/*case FIXED_ANGLE_CAMERA_MODE:
		gameState->m_world->m_player->SetCamera(this);
		m_attachedEntity = gameState->m_world->m_player;
		m_attachedEntity->EnableInput();
		break;*/
	/*case STATIONARY_CAMERA_MODE:
		m_attachedEntity->m_attachedCamera = nullptr;
		m_attachedEntity = nullptr;
		break;*/
		break;
	case DETACHED_CAMERA_MODE:
		m_attachedEntity->DisableInput();
		m_attachedEntity->m_attachedCamera = nullptr;
		m_attachedEntity = nullptr;	
		break;
	case NUM_CAMERA_MODES:
		break;
	}
}

//  =========================================================================================
std::string GameCamera::GetCameraModeAsText()
{
	std::string cameraModeAsText = "";
	switch (m_currentCameraMode)
	{
	case FIRST_PERSON_CAMERA_MODE:
		cameraModeAsText = "First Person";
		break;
	case THIRD_PERSON_CAMERA_MODE:
		cameraModeAsText = "Third Person";
		break;
	//case FIXED_ANGLE_CAMERA_MODE:
	//	cameraModeAsText = "Fixed Angle";
	//	break;
		/*case STATIONARY_CAMERA_MODE:
			cameraModeAsText = "Stationary";
		break;*/
	case DETACHED_CAMERA_MODE:
		cameraModeAsText = "Detached";
		break;
	case NUM_CAMERA_MODES:
		cameraModeAsText = "NOT SET";
		break;
	}

	return cameraModeAsText;
}
