#include "Game\Helpers\Ray.hpp"

//  =========================================================================================
Ray::Ray()
{
}

//  =========================================================================================
Ray::Ray(const Vector3& startPosition, const Vector3& direction, float maxDistance)
{
	m_startPosition = startPosition;
	m_direction = direction;
	m_maxDistance = maxDistance;
}

//  =========================================================================================
Ray::~Ray()
{
}
