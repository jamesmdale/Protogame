#pragma once
#include "Game\GameObjects\BlockLocator.hpp"
#include "Game\Helpers\Ray.hpp"
#include "Engine\Math\Vector3.hpp"

class RaycastResult
{
public:

	RaycastResult();
	RaycastResult(const Ray& ray, const BlockLocator& blockLocator, const bool didImpact, const Vector3& impactWorldPosition, const Vector3& endPosition, float impactDistance, const Vector3& impactNormal);
	~RaycastResult();

public:
	Ray m_ray; //start position, direction maxdistance y
	BlockLocator m_impactBlockLocator;
	bool m_didImpact = false;
	Vector3 m_impactWorldPosition;
	Vector3 m_endPosition;
	float m_impactDistance;
	Vector3 m_impactNormal;	
	float m_impactFraction;
	
};

