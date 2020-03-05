#pragma once
#include "Engine\Math\Vector3.hpp"
#include "Engine\Math\Matrix44.hpp"
#include <string>

class Entity;

enum CameraModes
{
	FIRST_PERSON_CAMERA_MODE,
	THIRD_PERSON_CAMERA_MODE,
	//FIXED_ANGLE_CAMERA_MODE,
	//STATIONARY_CAMERA_MODE,
	DETACHED_CAMERA_MODE,
	NUM_CAMERA_MODES
};

class GameCamera
{
public:
	GameCamera();
	~GameCamera();

	void UpdateFromInput(float deltaSeconds);

	void CreateFliippedViewMatrix(Matrix44& outMatrix);
	void Translate(const Vector3& translation);
	void AttachToEntity(Entity* entity);
	void SetRotations(float rollX, float pitchY, float yawZ);
	void SetTranslation(const Vector3& position);

	void CycleCameraModes();
	std::string GetCameraModeAsText();

public:
	float m_rollDegreesX = 0.0f; //roll
	float m_pitchDegreesY = 0.0f; //pitch
	float m_yawDegreesZ = 0.0f; //yaw

	CameraModes m_currentCameraMode = NUM_CAMERA_MODES;
	Vector3 m_position = Vector3::ZERO;
	Entity* m_attachedEntity = nullptr;
};

