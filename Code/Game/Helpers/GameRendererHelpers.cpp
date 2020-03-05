#include "Game\Helpers\GameRendererHelpers.hpp"
//#include "Engine\Renderer\Mesh.hpp"
//#include "Engine\Renderer\MeshBuilder.hpp"
#include "Engine\Math\Vector3.hpp"
#include "Game\GameCommon.hpp"

//  =========================================================================================
Mesh* CreateLine(const Vector3& start, const Vector3& end, const Rgba& color, float scale)
{
	MeshBuilder builder;

	builder.Begin(LINES_DRAW_PRIMITIVE, false);

	builder.SetColor(color);
	builder.SetUV(0, 0);
	builder.PushVertex(Vector3(start.x, start.y, start.z));

	builder.SetColor(color);
	builder.SetUV(0, 0);
	builder.PushVertex(Vector3(end.x, end.y, end.z));

	return builder.CreateMesh<VertexPCU>();
}

//  =========================================================================================
Mesh* CreateDebugStar(const Vector3& center, const Rgba& color, float scale)
{
	MeshBuilder builder;

	float halfScale = scale * 0.5f;

	builder.Begin(LINES_DRAW_PRIMITIVE, false);

	//east to west
	builder.SetColor(color);
	builder.SetUV(0, 0);
	builder.PushVertex(Vector3(center) + (g_east * scale));

	builder.SetColor(color);
	builder.SetUV(0, 0);
	builder.PushVertex(Vector3(center) + (g_west * scale));

	//north to south
	builder.SetColor(color);
	builder.SetUV(0, 0);
	builder.PushVertex(Vector3(center) + (g_north * scale));

	builder.SetColor(color);
	builder.SetUV(0, 0);
	builder.PushVertex(Vector3(center) + (g_south * scale));

	//up to down
	builder.SetColor(color);
	builder.SetUV(0, 0);
	builder.PushVertex(Vector3(center) + (g_worldUp * scale));

	builder.SetColor(color);
	builder.SetUV(0, 0);
	builder.PushVertex(Vector3(center) - (g_worldUp * scale));

	//diagonals
	builder.SetColor(color);
	builder.SetUV(0, 0);
	builder.PushVertex(Vector3(center) + (Vector3::ONE * halfScale));

	builder.SetColor(color);
	builder.SetUV(0, 0);
	builder.PushVertex(Vector3(center) - (Vector3::ONE * halfScale));

	//diagonals

	builder.SetColor(color);
	builder.SetUV(0, 0);
	builder.PushVertex(Vector3(center) + (Vector3(1.f, -1.f, -1.f) * halfScale));

	builder.SetColor(color);
	builder.SetUV(0, 0);
	builder.PushVertex(Vector3(center) - (Vector3(1.f, -1.f, -1.f) * halfScale));

	//diagonals
	builder.SetColor(color);
	builder.SetUV(0, 0);
	builder.PushVertex(Vector3(center) + (Vector3(-1.f, 1.f, -1.f) * halfScale));

	builder.SetColor(color);
	builder.SetUV(0, 0);
	builder.PushVertex(Vector3(center) - (Vector3(-1.f, 1.f, -1.f) * halfScale));

	

	return builder.CreateMesh<VertexPCU>();
}

//  =========================================================================================
void CreateDebugStar(MeshBuilder& builder, const Vector3& center, const Rgba& color, float scale)
{
	float halfScale = scale * 0.5f;

	builder.Begin(LINES_DRAW_PRIMITIVE, false);

	//east to west
	builder.SetColor(color);
	builder.SetUV(0, 0);
	builder.PushVertex(Vector3(center) + (g_east * scale));

	builder.SetColor(color);
	builder.SetUV(0, 0);
	builder.PushVertex(Vector3(center) + (g_west * scale));

	//north to south
	builder.SetColor(color);
	builder.SetUV(0, 0);
	builder.PushVertex(Vector3(center) + (g_north * scale));

	builder.SetColor(color);
	builder.SetUV(0, 0);
	builder.PushVertex(Vector3(center) + (g_south * scale));

	//up to down
	builder.SetColor(color);
	builder.SetUV(0, 0);
	builder.PushVertex(Vector3(center) + (g_worldUp * scale));

	builder.SetColor(color);
	builder.SetUV(0, 0);
	builder.PushVertex(Vector3(center) - (g_worldUp * scale));

	//diagonals
	builder.SetColor(color);
	builder.SetUV(0, 0);
	builder.PushVertex(Vector3(center) + (Vector3::ONE * halfScale));

	builder.SetColor(color);
	builder.SetUV(0, 0);
	builder.PushVertex(Vector3(center) - (Vector3::ONE * halfScale));

	//diagonals

	builder.SetColor(color);
	builder.SetUV(0, 0);
	builder.PushVertex(Vector3(center) + (Vector3(1.f, -1.f, -1.f) * halfScale));

	builder.SetColor(color);
	builder.SetUV(0, 0);
	builder.PushVertex(Vector3(center) - (Vector3(1.f, -1.f, -1.f) * halfScale));

	//diagonals
	builder.SetColor(color);
	builder.SetUV(0, 0);
	builder.PushVertex(Vector3(center) + (Vector3(-1.f, 1.f, -1.f) * halfScale));

	builder.SetColor(color);
	builder.SetUV(0, 0);
	builder.PushVertex(Vector3(center) - (Vector3(-1.f, 1.f, -1.f) * halfScale));
}

//  =========================================================================================
Mesh* CreateBlockHighlightBox(const Vector3& center, const Vector3& impactNormal, float scale)
{
	MeshBuilder builder;
	builder.Begin(TRIANGLES_DRAW_PRIMITIVE, true);
	
	int vertSize = builder.GetVertexCount();

	float xVal = 0.5f * scale;
	float yVal = 0.5f * scale;
	float zVal = 0.5f * scale;

	Rgba tint = Rgba(255, 255, 255, 50);
	
	//west face
	//front face
	if (impactNormal == g_west)
	{
		builder.SetColor(tint);
		builder.SetUV(0.f, 0.f);
		builder.PushVertex(Vector3(center.x - xVal, center.y + yVal, center.z - zVal));

		builder.SetColor(tint);
		builder.SetUV(0.f, 0.f);
		builder.PushVertex(Vector3(center.x - xVal, center.y - yVal, center.z - zVal));

		builder.SetColor(tint);
		builder.SetUV(0.f, 0.f);
		builder.PushVertex(Vector3(center.x - xVal, center.y - yVal, center.z + zVal));

		builder.SetColor(tint);
		builder.SetUV(0.f, 0.f);
		builder.PushVertex(Vector3(center.x - xVal, center.y + yVal, center.z + zVal));

		builder.AddQuadIndices(vertSize, vertSize + 1, vertSize + 2, vertSize + 3);
	}
	
	
	//south face
	//right face
	else if (impactNormal == g_south)
	{
		builder.SetColor(tint);
		builder.SetUV(0.f, 0.f);;
		builder.PushVertex(Vector3(center.x - xVal, center.y - yVal, center.z - zVal));

		builder.SetColor(tint);
		builder.SetUV(0.f, 0.f);
		builder.PushVertex(Vector3(center.x + xVal, center.y - yVal, center.z - zVal));

		builder.SetColor(tint);
		builder.SetUV(0.f, 0.f);
		builder.PushVertex(Vector3(center.x + xVal, center.y - yVal, center.z + zVal));

		builder.SetColor(tint);
		builder.SetUV(0.f, 0.f);
		builder.PushVertex(Vector3(center.x - xVal, center.y - yVal, center.z + zVal));

		builder.AddQuadIndices(vertSize, vertSize + 1, vertSize + 2, vertSize + 3);
	}
	

	//east face
	//back face	
	else if (impactNormal == g_east)
	{
		builder.SetColor(tint);
		builder.SetUV(0.f, 0.f);
		builder.PushVertex(Vector3(center.x + xVal, center.y - yVal, center.z - zVal));

		builder.SetColor(tint);
		builder.SetUV(0.f, 0.f);
		builder.PushVertex(Vector3(center.x + xVal, center.y + yVal, center.z - zVal));

		builder.SetColor(tint);
		builder.SetUV(0.f, 0.f);
		builder.PushVertex(Vector3(center.x + xVal, center.y + yVal, center.z + zVal));

		builder.SetColor(tint);
		builder.SetUV(0.f, 0.f);
		builder.PushVertex(Vector3(center.x + xVal, center.y - yVal, center.z + zVal));

		builder.AddQuadIndices(vertSize, vertSize + 1, vertSize + 2, vertSize + 3);
	}	
	
	//north face
	//left face
	else if (impactNormal == g_north)
	{
		builder.SetColor(tint);
		builder.SetUV(0.f, 0.f);
		builder.PushVertex(Vector3(center.x + xVal, center.y + yVal, center.z - zVal));

		builder.SetColor(tint);
		builder.SetUV(0.f, 0.f);
		builder.PushVertex(Vector3(center.x - xVal, center.y + yVal, center.z - zVal));

		builder.SetColor(tint);	
		builder.SetUV(0.f, 0.f);
		builder.PushVertex(Vector3(center.x - xVal, center.y + yVal, center.z + zVal));

		builder.SetColor(tint);
		builder.SetUV(0.f, 0.f);
		builder.PushVertex(Vector3(center.x + xVal, center.y + yVal, center.z + zVal));

		builder.AddQuadIndices(vertSize, vertSize + 1, vertSize + 2, vertSize + 3);
	}
	

	//up face
	//top face
	else if (impactNormal == g_worldUp)
	{
		builder.SetColor(tint);
		builder.SetUV(0.f, 0.f);
		builder.PushVertex(Vector3(center.x - xVal, center.y + yVal, center.z + zVal));

		builder.SetColor(tint);
		builder.SetUV(0.f, 0.f);
		builder.PushVertex(Vector3(center.x - xVal, center.y - yVal, center.z + zVal));

		builder.SetColor(tint);	
		builder.SetUV(0.f, 0.f);
		builder.PushVertex(Vector3(center.x + xVal, center.y - yVal, center.z + zVal));

		builder.SetColor(tint);
		builder.SetUV(0.f, 0.f);
		builder.PushVertex(Vector3(center.x + xVal, center.y + yVal, center.z + zVal));

		builder.AddQuadIndices(vertSize, vertSize + 1, vertSize + 2, vertSize + 3);
	}

	
	//down face
	//bottom face
	else if (impactNormal == -g_worldUp)
	{
		builder.SetColor(tint);
		builder.SetUV(0.f, 0.f);
		builder.PushVertex(Vector3(center.x + xVal, center.y + yVal, center.z - zVal));

		builder.SetColor(tint);
		builder.SetUV(0.f, 0.f);
		builder.PushVertex(Vector3(center.x + xVal, center.y - yVal, center.z - zVal));

		builder.SetColor(tint);	
		builder.SetUV(0.f, 0.f);
		builder.PushVertex(Vector3(center.x - xVal, center.y - yVal, center.z - zVal));

		builder.SetColor(tint);	
		builder.SetUV(0.f, 0.f);
		builder.PushVertex(Vector3(center.x - xVal, center.y + yVal, center.z - zVal));

		builder.AddQuadIndices(vertSize, vertSize + 1, vertSize + 2, vertSize + 3);
	}

	return builder.CreateMesh<VertexPCU>();
}

//  =========================================================================================
Mesh* CreateBlockHighlightBoxOutline(const Vector3& center, const Vector3& impactNormal, float scale)
{
	MeshBuilder builder;
	builder.Begin(LINES_DRAW_PRIMITIVE, false);

	int vertSize = builder.GetVertexCount();

	float xVal = 0.5f * scale;
	float yVal = 0.5f * scale;
	float zVal = 0.5f * scale;

	Rgba westTint = Rgba::WHITE;
	Rgba eastTint = Rgba::WHITE;
	Rgba northTint = Rgba::WHITE;
	Rgba southTint = Rgba::WHITE;
	Rgba topTint = Rgba::WHITE;
	Rgba bottomTint = Rgba::WHITE;

	//set the box color for the impact normal
	if (impactNormal == Vector3(-1.f, 0.f, 0.f))
		westTint = Rgba::PINK;
	else if (impactNormal == Vector3(1.f, 0.f, 0.f))
		eastTint = Rgba::PINK;
	else if (impactNormal == Vector3(0.f, 1.f, 0.f))
		northTint = Rgba::PINK;
	else if (impactNormal == Vector3(0.f, -1.f, 0.f))
		southTint = Rgba::PINK;
	else if (impactNormal == Vector3(0.f, 0.f, 1.f))
		topTint = Rgba::PINK;
	else if (impactNormal == Vector3(0.f, 0.f, -1.f))
		bottomTint = Rgba::PINK;

	//west face
	//front face	
	builder.SetColor(westTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x - xVal, center.y + yVal, center.z - zVal));

	builder.SetColor(westTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x - xVal, center.y - yVal, center.z - zVal));

	//  ----------------------------------------------

	builder.SetColor(westTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x - xVal, center.y - yVal, center.z - zVal));

	builder.SetColor(westTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x - xVal, center.y - yVal, center.z + zVal));

	//  ----------------------------------------------

	builder.SetColor(westTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x - xVal, center.y - yVal, center.z + zVal));

	builder.SetColor(westTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x - xVal, center.y + yVal, center.z + zVal));
	
	//  ----------------------------------------------

	builder.SetColor(westTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x - xVal, center.y + yVal, center.z + zVal));

	builder.SetColor(westTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x - xVal, center.y + yVal, center.z - zVal));

	//south face
	//right face
	builder.SetColor(southTint);
	builder.SetUV(0.f, 0.f);;
	builder.PushVertex(Vector3(center.x - xVal, center.y - yVal, center.z - zVal));

	builder.SetColor(southTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x + xVal, center.y - yVal, center.z - zVal));

	//  ----------------------------------------------

	builder.SetColor(southTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x + xVal, center.y - yVal, center.z - zVal));

	builder.SetColor(southTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x + xVal, center.y - yVal, center.z + zVal));
	
	//  ----------------------------------------------

	builder.SetColor(southTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x + xVal, center.y - yVal, center.z + zVal));

	builder.SetColor(southTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x - xVal, center.y - yVal, center.z + zVal));

	//  ----------------------------------------------

	builder.SetColor(southTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x - xVal, center.y - yVal, center.z + zVal));

	builder.SetColor(southTint);
	builder.SetUV(0.f, 0.f);;
	builder.PushVertex(Vector3(center.x - xVal, center.y - yVal, center.z - zVal));

	//east face
	//back face	
	builder.SetColor(eastTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x + xVal, center.y - yVal, center.z - zVal));

	builder.SetColor(eastTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x + xVal, center.y + yVal, center.z - zVal));

	//  ----------------------------------------------

	builder.SetColor(eastTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x + xVal, center.y + yVal, center.z - zVal));

	builder.SetColor(eastTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x + xVal, center.y + yVal, center.z + zVal));

	//  ----------------------------------------------

	builder.SetColor(eastTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x + xVal, center.y + yVal, center.z + zVal));

	builder.SetColor(eastTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x + xVal, center.y - yVal, center.z + zVal));

	//  ----------------------------------------------

	builder.SetColor(eastTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x + xVal, center.y - yVal, center.z + zVal));

	builder.SetColor(eastTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x + xVal, center.y - yVal, center.z - zVal));

	//north face
	//left face
	builder.SetColor(northTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x + xVal, center.y + yVal, center.z - zVal));

	builder.SetColor(northTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x - xVal, center.y + yVal, center.z - zVal));

	//  ----------------------------------------------

	builder.SetColor(northTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x - xVal, center.y + yVal, center.z - zVal));

	builder.SetColor(northTint);	
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x - xVal, center.y + yVal, center.z + zVal));

	//  ----------------------------------------------

	builder.SetColor(northTint);	
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x - xVal, center.y + yVal, center.z + zVal));

	builder.SetColor(northTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x + xVal, center.y + yVal, center.z + zVal));

	//  ----------------------------------------------

	builder.SetColor(northTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x + xVal, center.y + yVal, center.z + zVal));

	builder.SetColor(northTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x + xVal, center.y + yVal, center.z - zVal));

	//up face
	//top face
	builder.SetColor(topTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x - xVal, center.y + yVal, center.z + zVal));

	builder.SetColor(topTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x - xVal, center.y - yVal, center.z + zVal));

	//  ----------------------------------------------

	builder.SetColor(topTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x - xVal, center.y - yVal, center.z + zVal));

	builder.SetColor(topTint);	
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x + xVal, center.y - yVal, center.z + zVal));

	//  ----------------------------------------------

	builder.SetColor(topTint);	
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x + xVal, center.y - yVal, center.z + zVal));

	builder.SetColor(topTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x + xVal, center.y + yVal, center.z + zVal));

	//  ----------------------------------------------

	builder.SetColor(topTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x + xVal, center.y + yVal, center.z + zVal));

	builder.SetColor(topTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x - xVal, center.y + yVal, center.z + zVal));

	//down face
	//bottom face
	builder.SetColor(bottomTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x + xVal, center.y + yVal, center.z - zVal));

	builder.SetColor(bottomTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x + xVal, center.y - yVal, center.z - zVal));

	//  ----------------------------------------------

	builder.SetColor(bottomTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x + xVal, center.y - yVal, center.z - zVal));

	builder.SetColor(bottomTint);	
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x - xVal, center.y - yVal, center.z - zVal));

	//  ----------------------------------------------

	builder.SetColor(bottomTint);	
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x - xVal, center.y - yVal, center.z - zVal));

	builder.SetColor(bottomTint);	
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x - xVal, center.y + yVal, center.z - zVal));

	//  ----------------------------------------------

	builder.SetColor(bottomTint);	
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x - xVal, center.y + yVal, center.z - zVal));

	builder.SetColor(bottomTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x + xVal, center.y + yVal, center.z - zVal));


	//  create final mesh ----------------------------------------------
	return builder.CreateMesh<VertexPCU>();
}

//  =========================================================================================
Mesh* CreateGameBasis(const Vector3& center, float width, float scale)
{
	UNUSED(width);

	MeshBuilder builder;

	builder.Begin(TRIANGLES_DRAW_PRIMITIVE, true);

	//up - z
	builder.SetColor(Rgba::BLUE);
	builder.SetUV(0, 0);
	builder.PushVertex(Vector3(center.x, center.y, center.z));

	builder.SetColor(Rgba::BLUE);
	builder.SetUV(0, 0);
	builder.PushVertex(Vector3(center.x, center.y, center.z) + (scale * g_worldUp));

	builder.SetColor(Rgba::BLUE);
	builder.SetUV(0, 0);
	builder.PushVertex(Vector3(center.x, center.y, center.z));

	//forward - x
	builder.SetColor(Rgba::RED);
	builder.SetUV(0, 0);
	builder.PushVertex(Vector3(center.x, center.y, center.z));

	builder.SetColor(Rgba::RED);
	builder.SetUV(0, 0);
	builder.PushVertex(Vector3(center.x, center.y, center.z) + (scale * g_east));

	builder.SetColor(Rgba::RED);
	builder.SetUV(0, 0);
	builder.PushVertex(Vector3(center.x, center.y, center.z));

	//left - y
	builder.SetColor(Rgba::GREEN);
	builder.SetUV(0, 0);
	builder.PushVertex(Vector3(center.x, center.y, center.z));

	builder.SetColor(Rgba::GREEN);
	builder.SetUV(0, 0);
	builder.PushVertex(Vector3(center.x, center.y, center.z) + (scale * g_north));

	builder.SetColor(Rgba::GREEN);
	builder.SetUV(0, 0);
	builder.PushVertex(Vector3(center.x, center.y, center.z));

	return builder.CreateMesh<VertexPCU>();
}


//  =========================================================================================
Mesh* CreateBlockOutline(const Vector3& center, const Rgba& tint, float scale)
{
	MeshBuilder builder;
	builder.Begin(LINES_DRAW_PRIMITIVE, false);

	int vertSize = builder.GetVertexCount();

	float xVal = 0.5f * scale;
	float yVal = 0.5f * scale;
	float zVal = 0.5f * scale;

	Rgba westTint = tint;
	Rgba eastTint = tint;
	Rgba northTint = tint;
	Rgba southTint = tint;
	Rgba topTint = tint;
	Rgba bottomTint = tint;

	//west face
	//front face	
	builder.SetColor(westTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x - xVal, center.y + yVal, center.z - zVal));

	builder.SetColor(westTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x - xVal, center.y - yVal, center.z - zVal));

	//  ----------------------------------------------

	builder.SetColor(westTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x - xVal, center.y - yVal, center.z - zVal));

	builder.SetColor(westTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x - xVal, center.y - yVal, center.z + zVal));

	//  ----------------------------------------------

	builder.SetColor(westTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x - xVal, center.y - yVal, center.z + zVal));

	builder.SetColor(westTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x - xVal, center.y + yVal, center.z + zVal));

	//  ----------------------------------------------

	builder.SetColor(westTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x - xVal, center.y + yVal, center.z + zVal));

	builder.SetColor(westTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x - xVal, center.y + yVal, center.z - zVal));

	//south face
	//right face
	builder.SetColor(southTint);
	builder.SetUV(0.f, 0.f);;
	builder.PushVertex(Vector3(center.x - xVal, center.y - yVal, center.z - zVal));

	builder.SetColor(southTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x + xVal, center.y - yVal, center.z - zVal));

	//  ----------------------------------------------

	builder.SetColor(southTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x + xVal, center.y - yVal, center.z - zVal));

	builder.SetColor(southTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x + xVal, center.y - yVal, center.z + zVal));

	//  ----------------------------------------------

	builder.SetColor(southTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x + xVal, center.y - yVal, center.z + zVal));

	builder.SetColor(southTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x - xVal, center.y - yVal, center.z + zVal));

	//  ----------------------------------------------

	builder.SetColor(southTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x - xVal, center.y - yVal, center.z + zVal));

	builder.SetColor(southTint);
	builder.SetUV(0.f, 0.f);;
	builder.PushVertex(Vector3(center.x - xVal, center.y - yVal, center.z - zVal));

	//east face
	//back face	
	builder.SetColor(eastTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x + xVal, center.y - yVal, center.z - zVal));

	builder.SetColor(eastTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x + xVal, center.y + yVal, center.z - zVal));

	//  ----------------------------------------------

	builder.SetColor(eastTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x + xVal, center.y + yVal, center.z - zVal));

	builder.SetColor(eastTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x + xVal, center.y + yVal, center.z + zVal));

	//  ----------------------------------------------

	builder.SetColor(eastTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x + xVal, center.y + yVal, center.z + zVal));

	builder.SetColor(eastTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x + xVal, center.y - yVal, center.z + zVal));

	//  ----------------------------------------------

	builder.SetColor(eastTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x + xVal, center.y - yVal, center.z + zVal));

	builder.SetColor(eastTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x + xVal, center.y - yVal, center.z - zVal));

	//north face
	//left face
	builder.SetColor(northTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x + xVal, center.y + yVal, center.z - zVal));

	builder.SetColor(northTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x - xVal, center.y + yVal, center.z - zVal));

	//  ----------------------------------------------

	builder.SetColor(northTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x - xVal, center.y + yVal, center.z - zVal));

	builder.SetColor(northTint);	
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x - xVal, center.y + yVal, center.z + zVal));

	//  ----------------------------------------------

	builder.SetColor(northTint);	
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x - xVal, center.y + yVal, center.z + zVal));

	builder.SetColor(northTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x + xVal, center.y + yVal, center.z + zVal));

	//  ----------------------------------------------

	builder.SetColor(northTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x + xVal, center.y + yVal, center.z + zVal));

	builder.SetColor(northTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x + xVal, center.y + yVal, center.z - zVal));

	//up face
	//top face
	builder.SetColor(topTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x - xVal, center.y + yVal, center.z + zVal));

	builder.SetColor(topTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x - xVal, center.y - yVal, center.z + zVal));

	//  ----------------------------------------------

	builder.SetColor(topTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x - xVal, center.y - yVal, center.z + zVal));

	builder.SetColor(topTint);	
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x + xVal, center.y - yVal, center.z + zVal));

	//  ----------------------------------------------

	builder.SetColor(topTint);	
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x + xVal, center.y - yVal, center.z + zVal));

	builder.SetColor(topTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x + xVal, center.y + yVal, center.z + zVal));

	//  ----------------------------------------------

	builder.SetColor(topTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x + xVal, center.y + yVal, center.z + zVal));

	builder.SetColor(topTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x - xVal, center.y + yVal, center.z + zVal));

	//down face
	//bottom face
	builder.SetColor(bottomTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x + xVal, center.y + yVal, center.z - zVal));

	builder.SetColor(bottomTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x + xVal, center.y - yVal, center.z - zVal));

	//  ----------------------------------------------

	builder.SetColor(bottomTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x + xVal, center.y - yVal, center.z - zVal));

	builder.SetColor(bottomTint);	
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x - xVal, center.y - yVal, center.z - zVal));

	//  ----------------------------------------------

	builder.SetColor(bottomTint);	
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x - xVal, center.y - yVal, center.z - zVal));

	builder.SetColor(bottomTint);	
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x - xVal, center.y + yVal, center.z - zVal));

	//  ----------------------------------------------

	builder.SetColor(bottomTint);	
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x - xVal, center.y + yVal, center.z - zVal));

	builder.SetColor(bottomTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x + xVal, center.y + yVal, center.z - zVal));


	//  create final mesh ----------------------------------------------
	return builder.CreateMesh<VertexPCU>();
}

//  =========================================================================================
Mesh* CreateCubeOutline(const AABB3& bounds, const Rgba& tint, float scale)
{
	MeshBuilder builder;
	builder.Begin(LINES_DRAW_PRIMITIVE, false);

	int vertSize = builder.GetVertexCount();

	Vector3 center = bounds.GetCenter();
	float xVal = 0.5f * bounds.GetDimensions().x * scale;
	float yVal = 0.5f * bounds.GetDimensions().y * scale;
	float zVal = 0.5f * bounds.GetDimensions().z * scale;

	Rgba westTint = tint;
	Rgba eastTint = tint;
	Rgba northTint = tint;
	Rgba southTint = tint;
	Rgba topTint = tint;
	Rgba bottomTint = tint;

	//west face
	//front face	
	builder.SetColor(westTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x - xVal, center.y + yVal, center.z - zVal));

	builder.SetColor(westTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x - xVal, center.y - yVal, center.z - zVal));

	//  ----------------------------------------------

	builder.SetColor(westTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x - xVal, center.y - yVal, center.z - zVal));

	builder.SetColor(westTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x - xVal, center.y - yVal, center.z + zVal));

	//  ----------------------------------------------

	builder.SetColor(westTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x - xVal, center.y - yVal, center.z + zVal));

	builder.SetColor(westTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x - xVal, center.y + yVal, center.z + zVal));

	//  ----------------------------------------------

	builder.SetColor(westTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x - xVal, center.y + yVal, center.z + zVal));

	builder.SetColor(westTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x - xVal, center.y + yVal, center.z - zVal));

	//south face
	//right face
	builder.SetColor(southTint);
	builder.SetUV(0.f, 0.f);;
	builder.PushVertex(Vector3(center.x - xVal, center.y - yVal, center.z - zVal));

	builder.SetColor(southTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x + xVal, center.y - yVal, center.z - zVal));

	//  ----------------------------------------------

	builder.SetColor(southTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x + xVal, center.y - yVal, center.z - zVal));

	builder.SetColor(southTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x + xVal, center.y - yVal, center.z + zVal));

	//  ----------------------------------------------

	builder.SetColor(southTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x + xVal, center.y - yVal, center.z + zVal));

	builder.SetColor(southTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x - xVal, center.y - yVal, center.z + zVal));

	//  ----------------------------------------------

	builder.SetColor(southTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x - xVal, center.y - yVal, center.z + zVal));

	builder.SetColor(southTint);
	builder.SetUV(0.f, 0.f);;
	builder.PushVertex(Vector3(center.x - xVal, center.y - yVal, center.z - zVal));

	//east face
	//back face	
	builder.SetColor(eastTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x + xVal, center.y - yVal, center.z - zVal));

	builder.SetColor(eastTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x + xVal, center.y + yVal, center.z - zVal));

	//  ----------------------------------------------

	builder.SetColor(eastTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x + xVal, center.y + yVal, center.z - zVal));

	builder.SetColor(eastTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x + xVal, center.y + yVal, center.z + zVal));

	//  ----------------------------------------------

	builder.SetColor(eastTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x + xVal, center.y + yVal, center.z + zVal));

	builder.SetColor(eastTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x + xVal, center.y - yVal, center.z + zVal));

	//  ----------------------------------------------

	builder.SetColor(eastTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x + xVal, center.y - yVal, center.z + zVal));

	builder.SetColor(eastTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x + xVal, center.y - yVal, center.z - zVal));

	//north face
	//left face
	builder.SetColor(northTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x + xVal, center.y + yVal, center.z - zVal));

	builder.SetColor(northTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x - xVal, center.y + yVal, center.z - zVal));

	//  ----------------------------------------------

	builder.SetColor(northTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x - xVal, center.y + yVal, center.z - zVal));

	builder.SetColor(northTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x - xVal, center.y + yVal, center.z + zVal));

	//  ----------------------------------------------

	builder.SetColor(northTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x - xVal, center.y + yVal, center.z + zVal));

	builder.SetColor(northTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x + xVal, center.y + yVal, center.z + zVal));

	//  ----------------------------------------------

	builder.SetColor(northTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x + xVal, center.y + yVal, center.z + zVal));

	builder.SetColor(northTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x + xVal, center.y + yVal, center.z - zVal));

	//up face
	//top face
	builder.SetColor(topTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x - xVal, center.y + yVal, center.z + zVal));

	builder.SetColor(topTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x - xVal, center.y - yVal, center.z + zVal));

	//  ----------------------------------------------

	builder.SetColor(topTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x - xVal, center.y - yVal, center.z + zVal));

	builder.SetColor(topTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x + xVal, center.y - yVal, center.z + zVal));

	//  ----------------------------------------------

	builder.SetColor(topTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x + xVal, center.y - yVal, center.z + zVal));

	builder.SetColor(topTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x + xVal, center.y + yVal, center.z + zVal));

	//  ----------------------------------------------

	builder.SetColor(topTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x + xVal, center.y + yVal, center.z + zVal));

	builder.SetColor(topTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x - xVal, center.y + yVal, center.z + zVal));

	//down face
	//bottom face
	builder.SetColor(bottomTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x + xVal, center.y + yVal, center.z - zVal));

	builder.SetColor(bottomTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x + xVal, center.y - yVal, center.z - zVal));

	//  ----------------------------------------------

	builder.SetColor(bottomTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x + xVal, center.y - yVal, center.z - zVal));

	builder.SetColor(bottomTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x - xVal, center.y - yVal, center.z - zVal));

	//  ----------------------------------------------

	builder.SetColor(bottomTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x - xVal, center.y - yVal, center.z - zVal));

	builder.SetColor(bottomTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x - xVal, center.y + yVal, center.z - zVal));

	//  ----------------------------------------------

	builder.SetColor(bottomTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x - xVal, center.y + yVal, center.z - zVal));

	builder.SetColor(bottomTint);
	builder.SetUV(0.f, 0.f);
	builder.PushVertex(Vector3(center.x + xVal, center.y + yVal, center.z - zVal));


	//  create final mesh ----------------------------------------------
	return builder.CreateMesh<VertexPCU>();
}

//  =========================================================================================
Mesh* CreateGameUVSphere(const Sphere& sphere, const Rgba& tint, float scale)
{
	MeshBuilder builder;
	Vector3 position = sphere.m_position;
	float radius = sphere.m_radius;
	int wedges = 30;
	int slices = 30;

	//(0,0) to (1,1)
	builder.Begin(TRIANGLES_DRAW_PRIMITIVE, true); //begin is does use index buffer

	int vertSize = builder.GetVertexCount();

	for (int sliceIndex = 0; sliceIndex <= slices; ++sliceIndex)
	{
		float v = (float)sliceIndex / ((float)slices - 1.f);

		float azimuth = RangeMapFloat(v, 0.f, 1.f, -90.f, 90.f);

		for (int wedgeIndex = 0; wedgeIndex <= wedges; ++wedgeIndex)
		{
			float u = (float)wedgeIndex / ((float)wedges);
			float rotation = ConvertRadiansToDegrees(2.0f * GetPi() * u); //or rangemap

			builder.SetUV(Vector2(u, v));
			builder.SetColor(tint);
			Vector3 vertexPos = position + SphericalToCartesian(radius, rotation, azimuth);

			Vector3 normal = vertexPos - position;
			normal.Normalize();

			//derivative of (tan = d/du Polar)
			float x = -1.f * CosDegrees(azimuth) * SinDegrees(rotation) * radius;
			float y = 0.f;
			float z = CosDegrees(azimuth) * CosDegrees(rotation) * radius;

			Vector4 tangent = Vector4(x, y, z, 1.f);
			tangent.Normalize();

			builder.SetTangent(tangent);
			builder.SetNormal(normal);
			builder.PushVertex(vertexPos);
		}
	}

	//now that we have all the verts...we need to construct faces via indices
	//best to think of vertexes as a 2D array
	for (int sliceIndex = 0; sliceIndex < slices; ++sliceIndex)
	{
		for (int wedgeIndex = 0; wedgeIndex < wedges; ++wedgeIndex)
		{
			int bl_idx = wedges * sliceIndex + wedgeIndex;
			int tl_idx = bl_idx + wedges;
			int br_idx = bl_idx + 1;
			int tr_idx = br_idx + wedges;

			builder.AddQuadIndices(vertSize + bl_idx, vertSize + br_idx, vertSize + tr_idx, vertSize + tl_idx);
		}
	}

	//  create final mesh ----------------------------------------------
	return builder.CreateMesh<VertexPCU>();
}

