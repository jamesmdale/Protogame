#include "Game\GameObjects\Entity.hpp"
#include "Game\GameObjects\GameCamera.hpp"
#include "Game\GameObjects\World.hpp"

//  =========================================================================================
Entity::Entity(World* world)
{
	m_world = world;
}

//  =========================================================================================
Entity::~Entity()
{
	m_attachedCamera = nullptr;
	m_world = nullptr;
}

//  =========================================================================================
void Entity::Update(float deltaSeconds)
{
	//does nothing. Intended for derivation
}

//  =========================================================================================
float Entity::UpdateFromInput(float deltaSeconds)
{
	return deltaSeconds;
}


//  =========================================================================================
void Entity::UpdatePhysics(float deltaSeconds)
{
	UNUSED(deltaSeconds);
}

//  =========================================================================================
void Entity::Render()
{
	//does nothing. Intended for derivation
}

//  =========================================================================================
void Entity::SetCamera(GameCamera* camera)
{
	m_attachedCamera = camera;
}

//  =========================================================================================
void Entity::DetachCamera()
{
	m_attachedCamera = nullptr;
}

//  =========================================================================================
void Entity::CyclePhysicsModes()
{
	m_currentPhysicsMode = (PhysicsMode)((int)m_currentPhysicsMode + 1);

	if (m_currentPhysicsMode == NUM_PHYSICS_MODES)
		m_currentPhysicsMode = (PhysicsMode)0;
}

//  =========================================================================================
std::string Entity::GetPhysicsModeAsText()
{
	std::string physicsModeAsText = "";
	switch (m_currentPhysicsMode)
	{
	case WALKING_PHYSICS_MODE:
		physicsModeAsText = "Walking";
		break;
	case FLYING_PHYSICS_MODE:
		physicsModeAsText = "Flying";
		break;
	case NO_CLIP_PHYSICS_MODE:
		physicsModeAsText = "NoClip";
		break;
	case NUM_PHYSICS_MODES:
		physicsModeAsText = "NOT SET";
		break;
	}

	return physicsModeAsText;
}

//  =========================================================================================
Vector3 Entity::GetForward()
{
	Vector3 entityForward = Vector3(CosDegrees(m_yawDegreesZ) * CosDegrees(m_pitchDegreesY)
		, SinDegrees(m_yawDegreesZ) * CosDegrees(m_pitchDegreesY),
		-1.f * SinDegrees(m_pitchDegreesY));

	return entityForward;
}

