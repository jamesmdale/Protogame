#include "Game\Helpers\RaycastResult.hpp"


//  =========================================================================================
RaycastResult::RaycastResult()
{
}

// =========================================================================================
RaycastResult::RaycastResult(const Ray& ray, const BlockLocator& blockLocator, const bool didImpact, const Vector3& impactWorldPosition, const Vector3& endPosition, float impactDistance, const Vector3& impactNormal)
{
	m_ray = ray;
	m_impactBlockLocator =  blockLocator;
	m_didImpact = didImpact;
	m_impactWorldPosition = impactWorldPosition;
	m_endPosition = endPosition;
	m_impactNormal = impactNormal;
	m_impactDistance = impactDistance;
	m_impactFraction = m_impactDistance / ray.m_maxDistance;	
}

//  =========================================================================================
RaycastResult::~RaycastResult()
{
}
