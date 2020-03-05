#include "Game\ObjectMeshbuilder.hpp"
#include "Game\Definitions\BlockDefinition.hpp"
#include "Game\Game.hpp"
#include "Engine\Renderer\MeshBuilder.hpp"

//  =========================================================================================
Mesh* MakeBlockToMesh(const Vector3& center, const uint8 type)
{
	MeshBuilder builder;

	float xVal = 0.5f;
	float yVal = 0.5f;
	float zVal = 0.5f;

	BlockDefinition* blockDef = BlockDefinition::GetDefinitionById(type);

	AABB2 frontTexCoords = GetTerrainSprites()->GetTexCoordsForSpriteCoords(blockDef->m_frontTexCoords);
	AABB2 rightTexCoords = GetTerrainSprites()->GetTexCoordsForSpriteCoords(blockDef->m_rightSideTexCoords);
	AABB2 backTexCoords = GetTerrainSprites()->GetTexCoordsForSpriteCoords(blockDef->m_backTexCoords);
	AABB2 leftTexCoords = GetTerrainSprites()->GetTexCoordsForSpriteCoords(blockDef->m_leftSideTexCoords);
	AABB2 topTexCoords = GetTerrainSprites()->GetTexCoordsForSpriteCoords(blockDef->m_topTexCoords);
	AABB2 bottomTexCoords = GetTerrainSprites()->GetTexCoordsForSpriteCoords(blockDef->m_bottomTexCoords);

	Rgba tint = Rgba::WHITE;

	int vertSize = builder.GetVertexCount();

	builder.Begin(TRIANGLES_DRAW_PRIMITIVE, true); //begin is does use index buffer

												   //front face
	builder.SetColor(tint);
	builder.SetUV(frontTexCoords.maxs.x, frontTexCoords.maxs.y);
	builder.SetNormal(Vector3(0.f, 0.f, -1.f));
	builder.SetTangent(Vector4(1.f, 0.f, 0.f, 1.f));
	builder.PushVertex(Vector3(center.x - xVal, center.y + yVal, center.z - zVal));

	builder.SetColor(tint);
	builder.SetUV(frontTexCoords.mins.x, frontTexCoords.maxs.y);
	builder.SetNormal(Vector3(0.f, 0.f, -1.f));
	builder.SetTangent(Vector4(1.f, 0.f, 0.f, 1.f));
	builder.PushVertex(Vector3(center.x - xVal, center.y - yVal, center.z - zVal));

	builder.SetColor(tint);	
	builder.SetUV(frontTexCoords.mins.x, frontTexCoords.mins.y);
	builder.SetNormal(Vector3(0.f, 0.f, -1.f));
	builder.SetTangent(Vector4(1.f, 0.f, 0.f, 1.f));
	builder.PushVertex(Vector3(center.x - xVal, center.y - yVal, center.z + zVal));

	builder.SetColor(tint);	
	builder.SetUV(frontTexCoords.maxs.x, frontTexCoords.mins.y);
	builder.SetNormal(Vector3(0.f, 0.f, -1.f));
	builder.SetTangent(Vector4(1.f, 0.f, 0.f, 1.f));
	builder.PushVertex(Vector3(center.x - xVal, center.y + yVal, center.z + zVal));

	builder.AddQuadIndices(vertSize, vertSize + 1, vertSize + 2, vertSize + 3);

	//right face
	builder.SetColor(tint);
	builder.SetUV(rightTexCoords.maxs.x, rightTexCoords.maxs.y);
	builder.SetNormal(Vector3(1.f, 0.f, 0.f));
	builder.SetTangent(Vector4(0.f, 0.f, -1.f, 1.f));
	builder.PushVertex(Vector3(center.x - xVal, center.y - yVal, center.z - zVal));

	builder.SetColor(tint);
	builder.SetUV(rightTexCoords.mins.x, rightTexCoords.maxs.y);
	builder.SetNormal(Vector3(1.f, 0.f, 0.f));
	builder.SetTangent(Vector4(0.f, 0.f, -1.f, 1.f));
	builder.PushVertex(Vector3(center.x + xVal, center.y - yVal, center.z - zVal));

	builder.SetColor(tint);
	builder.SetUV(rightTexCoords.mins.x, rightTexCoords.mins.y);
	builder.SetNormal(Vector3(1.f, 0.f, 0.f));
	builder.SetTangent(Vector4(0.f, 0.f, -1.f, 1.f));
	builder.PushVertex(Vector3(center.x + xVal, center.y - yVal, center.z + zVal));

	builder.SetColor(tint);	
	builder.SetUV(rightTexCoords.maxs.x, rightTexCoords.mins.y);
	builder.SetNormal(Vector3(1.f, 0.f, 0.f));
	builder.SetTangent(Vector4(0.f, 0.f, -1.f, 1.f));
	builder.PushVertex(Vector3(center.x - xVal, center.y - yVal, center.z + zVal));

	builder.AddQuadIndices(vertSize + 4, vertSize + 5, vertSize + 6, vertSize + 7);

	//back face
	builder.SetColor(tint);
	builder.SetUV(backTexCoords.maxs.x, backTexCoords.maxs.y);
	builder.SetNormal(Vector3(0.f, 0.f, 1.f));
	builder.SetTangent(Vector4(-1.f, 0.f, 0.f, 1.f));
	builder.PushVertex(Vector3(center.x + xVal, center.y - yVal, center.z - zVal));

	builder.SetColor(tint);
	builder.SetUV(backTexCoords.mins.x, backTexCoords.maxs.y);
	builder.SetNormal(Vector3(0.f, 0.f, 1.f));
	builder.SetTangent(Vector4(-1.f, 0.f, 0.f, 1.f));
	builder.PushVertex(Vector3(center.x + xVal, center.y + yVal, center.z - zVal));

	builder.SetColor(tint);
	builder.SetUV(backTexCoords.mins.x, backTexCoords.mins.y);
	builder.SetNormal(Vector3(0.f, 0.f, 1.f));
	builder.SetTangent(Vector4(-1.f, 0.f, 0.f, 1.f));
	builder.PushVertex(Vector3(center.x + xVal, center.y + yVal, center.z + zVal));

	builder.SetColor(tint);	
	builder.SetUV(backTexCoords.maxs.x, backTexCoords.mins.y);
	builder.SetNormal(Vector3(0.f, 0.f, 1.f));
	builder.SetTangent(Vector4(-1.f, 0.f, 0.f, 1.f));
	builder.PushVertex(Vector3(center.x + xVal, center.y - yVal, center.z + zVal));

	builder.AddQuadIndices(vertSize + 8, vertSize + 9, vertSize + 10, vertSize + 11);

	//left face
	builder.SetColor(tint);
	builder.SetUV(leftTexCoords.maxs.x, leftTexCoords.maxs.y);
	builder.SetNormal(Vector3(-1.f, 0.f, 0.f));
	builder.SetTangent(Vector4(0.f, 0.f, 1.f, 1.f));
	builder.PushVertex(Vector3(center.x + xVal, center.y + yVal, center.z - zVal));

	builder.SetColor(tint);
	builder.SetUV(leftTexCoords.mins.x, leftTexCoords.maxs.y);
	builder.SetNormal(Vector3(-1.f, 0.f, 0.f));
	builder.SetTangent(Vector4(0.f, 0.f, 1.f, 1.f));
	builder.PushVertex(Vector3(center.x - xVal, center.y + yVal, center.z - zVal));

	builder.SetColor(tint);
	builder.SetUV(leftTexCoords.mins.x, leftTexCoords.mins.y);
	builder.SetNormal(Vector3(-1.f, 0.f, 0.f));
	builder.SetTangent(Vector4(0.f, 0.f, 1.f, 1.f));
	builder.PushVertex(Vector3(center.x - xVal, center.y + yVal, center.z + zVal));

	builder.SetColor(tint);
	builder.SetUV(leftTexCoords.maxs.x, leftTexCoords.mins.y);
	builder.SetNormal(Vector3(-1.f, 0.f, 0.f));
	builder.SetTangent(Vector4(0.f, 0.f, 1.f, 1.f));
	builder.PushVertex(Vector3(center.x + xVal, center.y + yVal, center.z + zVal));

	builder.AddQuadIndices(vertSize + 12, vertSize + 13, vertSize + 14, vertSize + 15);

	//top face
	builder.SetColor(tint);
	builder.SetUV(topTexCoords.maxs.x, topTexCoords.maxs.y);
	builder.SetNormal(Vector3(0.f, 1.f, 0.f));
	builder.SetTangent(Vector4(1.f, 0.f, 0.f, 1.f));
	builder.PushVertex(Vector3(center.x - xVal, center.y + yVal, center.z + zVal));

	builder.SetColor(tint);
	builder.SetUV(topTexCoords.mins.x, topTexCoords.maxs.y);
	builder.SetNormal(Vector3(0.f, 1.f, 0.f));
	builder.SetTangent(Vector4(1.f, 0.f, 0.f, 1.f));
	builder.PushVertex(Vector3(center.x - xVal, center.y - yVal, center.z + zVal));

	builder.SetColor(tint);
	builder.SetUV(topTexCoords.mins.x, topTexCoords.mins.y);
	builder.SetNormal(Vector3(0.f, 1.f, 0.f));
	builder.SetTangent(Vector4(1.f, 0.f, 0.f, 1.f));
	builder.PushVertex(Vector3(center.x + xVal, center.y - yVal, center.z + zVal));

	builder.SetColor(tint);
	builder.SetUV(topTexCoords.maxs.x, topTexCoords.mins.y);
	builder.SetNormal(Vector3(0.f, 1.f, 0.f));
	builder.SetTangent(Vector4(1.f, 0.f, 0.f, 1.f));
	builder.PushVertex(Vector3(center.x + xVal, center.y + yVal, center.z + zVal));

	builder.AddQuadIndices(vertSize + 16, vertSize + 17, vertSize + 18, vertSize + 19);

	//bottom face
	builder.SetColor(tint);

	builder.SetUV(bottomTexCoords.maxs.x, bottomTexCoords.maxs.y);
	builder.SetNormal(Vector3(0.f, -1.f, 0.f));
	builder.SetTangent(Vector4(-1.f, 0.f, 0.f, 1.f));
	builder.PushVertex(Vector3(center.x + xVal, center.y + yVal, center.z - zVal));

	builder.SetColor(tint);
	builder.SetUV(bottomTexCoords.mins.x, bottomTexCoords.maxs.y);
	builder.SetNormal(Vector3(0.f, -1.f, 0.f));
	builder.SetTangent(Vector4(-1.f, 0.f, 0.f, 1.f));
	builder.PushVertex(Vector3(center.x + xVal, center.y - yVal, center.z - zVal));

	builder.SetColor(tint);
	builder.SetUV(bottomTexCoords.mins.x, bottomTexCoords.mins.y);
	builder.SetNormal(Vector3(0.f, -1.f, 0.f));
	builder.SetTangent(Vector4(-1.f, 0.f, 0.f, 1.f));
	builder.PushVertex(Vector3(center.x - xVal, center.y - yVal, center.z - zVal));

	builder.SetColor(tint);	
	builder.SetUV(bottomTexCoords.maxs.x, bottomTexCoords.mins.y);
	builder.SetNormal(Vector3(0.f, -1.f, 0.f));
	builder.SetTangent(Vector4(-1.f, 0.f, 0.f, 1.f));
	builder.PushVertex(Vector3(center.x - xVal, center.y + yVal, center.z - zVal));

	builder.AddQuadIndices(vertSize + 20, vertSize + 21, vertSize + 22, vertSize + 23);

	//generate mesh and return
	return builder.CreateMesh<VertexPCU>();
}
