#pragma once
#include "Engine\Math\AABB3.hpp"
#include "Engine\Math\Sphere.hpp"
#include <string>

class GameCamera;
class World;

enum PhysicsMode
{
	WALKING_PHYSICS_MODE,
	FLYING_PHYSICS_MODE,
	NO_CLIP_PHYSICS_MODE,
	NUM_PHYSICS_MODES
};

class Entity
{
public:
	Entity(World* world);
	virtual ~Entity();

	virtual void Update(float deltaSeconds);
	virtual float UpdateFromInput(float deltaSeconds);
	virtual void UpdatePhysics(float deltaSeconds);
	virtual void Render();
	
	void SetCamera(GameCamera* camera);
	void DetachCamera();

	void CyclePhysicsModes();
	std::string GetPhysicsModeAsText();

	inline bool IsCameraAttached() { return m_attachedCamera != nullptr; }
	inline void EnableInput() { m_doesReceiveUserInput = true; }
	inline void DisableInput() { m_doesReceiveUserInput = false; }
	inline bool DoesReceiveInput() { return m_doesReceiveUserInput; }
	inline Sphere GetPhysicsBounds() { return m_physicsSphere; }
	inline void Translate(const Vector3& translation) { m_position += translation; }

	Vector3 GetForward();

public:
	Vector3 m_firstPersonCameraPositionOffsetFromPivot = Vector3::ZERO;
	
	Vector3 m_position = Vector3::ZERO;	
	Vector3 m_velocity = Vector3::ZERO;
	Vector3 m_frameMoveIntention = Vector3::ZERO;

	float m_rollDegreesX = 0.0f; //roll
	float m_pitchDegreesY = 0.0f; //pitch
	float m_yawDegreesZ = 0.0f; //yaw

	//world reference
	World* m_world = nullptr;
	PhysicsMode m_currentPhysicsMode = WALKING_PHYSICS_MODE;

public:
	Sphere m_physicsSphere = Sphere(Vector3::ZERO, 0.f);
	bool m_doesReceiveUserInput = false;
	GameCamera* m_attachedCamera = nullptr;
};