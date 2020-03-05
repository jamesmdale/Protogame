#pragma once
#include "Engine\Math\Vector3.hpp"
#include "Engine\Math\AABB3.hpp"
#include "Engine\Math\Sphere.hpp"
#include "Engine\Core\Rgba.hpp"
#include "Engine\Renderer\MeshBuilder.hpp"
#include "Engine\Renderer\Mesh.hpp"

//class Mesh;

//custom render functions
Mesh* CreateLine(const Vector3& start, const Vector3& end, const Rgba& color = Rgba::WHITE, float scale = 1.f);
Mesh* CreateDebugStar(const Vector3& center, const Rgba& color, float scale);
void CreateDebugStar(MeshBuilder& builder, const Vector3& center, const Rgba& color, float scale);
Mesh* CreateBlockHighlightBox(const Vector3& center, const Vector3& impactNormal, float scale = 1.1f);
Mesh* CreateBlockHighlightBoxOutline(const Vector3& center, const Vector3& impactNormal, float scale = 1.1f);
Mesh* CreateGameBasis(const Vector3& center, float width, float scale = 1.f);
Mesh* CreateBlockOutline(const Vector3& center, const Rgba& tint = Rgba::WHITE, float scale = 1.f);
Mesh* CreateCubeOutline(const AABB3& bounds, const Rgba& tint = Rgba::WHITE, float scale = 1.f);
Mesh* CreateGameUVSphere(const Sphere& sphere, const Rgba& tint = Rgba::WHITE, float scale = 1.f);


