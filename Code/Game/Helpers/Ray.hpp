#pragma once
#include "Engine\Math\Vector3.hpp"

class Ray
{
public:
	Ray();
	Ray(const Vector3& startPosition, const Vector3& direction, float maxDistance);
	~Ray();

public:
	Vector3 m_startPosition = Vector3::ONE;
	Vector3 m_direction = Vector3::FORWARD;
	float m_maxDistance = 1.f;
};

