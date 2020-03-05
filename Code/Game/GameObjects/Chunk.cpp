#include "Game\GameObjects\Chunk.hpp"
#include "Game\Definitions\BlockDefinition.hpp"
#include "Game\GameCommon.hpp"
#include "Game\Game.hpp"
#include "Game\GameObjects\BlockLocator.hpp"
#include "Engine\Renderer\MeshBuilder.hpp"
#include "Engine\Renderer\Mesh.hpp"
#include "Engine\Math\SmoothNoise.hpp"
#include "Engine\Math\MathUtils.hpp"
#include "Engine\Renderer\Material.hpp"
#include "Engine\Renderer\Shader.hpp"
#include "Engine\Renderer\ShaderProgram.hpp"


const Vector3 blockCenterOffset = Vector3(0.5f, 0.5f, 0.5f);

//  =========================================================================================
Chunk::Chunk(const IntVector2& coordinates)
{
	m_chunkCoords = coordinates;

	BlockDefinition* airBlockDef = BlockDefinition::GetDefinitionById(AIR_BLOCK_ID);
	for (int blockIndex = 0; blockIndex < BLOCKS_PER_CHUNK; ++blockIndex)
	{
		m_blocks[blockIndex].m_type = airBlockDef->m_type;
	}

	Vector3 minsWorldCoordinates = Vector3((float)m_chunkCoords.x * BLOCKS_WIDE_X, (float)m_chunkCoords.y * BLOCKS_WIDE_Y, 0.f);
	Vector3 maxsWorldCoordinates = minsWorldCoordinates + Vector3(BLOCKS_WIDE_X, BLOCKS_WIDE_Y, BLOCKS_HIGH_Z);

	m_worldBounds = AABB3(minsWorldCoordinates, maxsWorldCoordinates);

	m_meshBuilder = new MeshBuilder();
}

//  =========================================================================================
Chunk::~Chunk()
{
	delete(m_meshBuilder);
	m_meshBuilder = nullptr;

	delete(m_gpuMesh);
	m_gpuMesh = nullptr;

	//clean up neighbor pointers
	m_northNeighbor = nullptr;
	m_westNeighbor = nullptr;
	m_southNeighbor = nullptr;
	m_eastNeighbor = nullptr;
	
}

//  =========================================================================================
void Chunk::Update()
{

}

//  =========================================================================================
void Chunk::Render()
{
	Renderer* theRenderer = Renderer::GetInstance();
	theRenderer->DrawMesh(m_gpuMesh);
}

//  =========================================================================================
void Chunk::GenerateBlockDataWithPerlin()
{
	std::vector<int> columnHeights;

	//foreach column we need to generate a height
	for (int columnIndex = 0; columnIndex < BLOCKS_WIDE_X * BLOCKS_WIDE_Y; ++columnIndex)
	{
		IntVector3 columnCoordinates = GetBlockCoordsForBlockIndex(columnIndex);
		Vector3 columnWorldCoordinates = GetBlockWorldCenterForBlockIndex(columnIndex);
		float height0to1 = Compute2dPerlinNoise(columnWorldCoordinates.x, columnWorldCoordinates.y, 300.f, 5);

		float height = RangeMapFloat(height0to1, 0.f, 1.f, 100.f, 150.f);

		int heightAsInt = RoundToNearestInt(height);

		for (int blockIndexInColumn = 0; blockIndexInColumn < BLOCKS_HIGH_Z; ++blockIndexInColumn)
		{	
			//leave as air
			if (blockIndexInColumn > heightAsInt)
			{
				//do nothing. all of this is air
			}

			//make the block grass if it is on top
			else if (blockIndexInColumn == heightAsInt)
			{
				BlockDefinition* blockDefinitionForType = BlockDefinition::GetDefinitionById(GRASS_BLOCK_ID);
				m_blocks[GetBlockIndexForBlockCoords(IntVector3(columnCoordinates.x, columnCoordinates.y, blockIndexInColumn))].m_type = blockDefinitionForType->m_type;
				m_blocks[GetBlockIndexForBlockCoords(IntVector3(columnCoordinates.x, columnCoordinates.y, blockIndexInColumn))].m_bits = blockDefinitionForType->m_defaultBits;
				//BlockDefinition::UpdateBitsBasedOnType(block.m_type, &block.m_bits);
			}

			//else make the block dirt
			else if (blockIndexInColumn < heightAsInt && blockIndexInColumn >= heightAsInt - 3)
			{
				BlockDefinition* blockDefinitionForType = BlockDefinition::GetDefinitionById(DIRT_BLOCK_ID);
				m_blocks[GetBlockIndexForBlockCoords(IntVector3(columnCoordinates.x, columnCoordinates.y, blockIndexInColumn))].m_type = blockDefinitionForType->m_type;
				m_blocks[GetBlockIndexForBlockCoords(IntVector3(columnCoordinates.x, columnCoordinates.y, blockIndexInColumn))].m_bits = blockDefinitionForType->m_defaultBits;
				//BlockDefinition::UpdateBitsBasedOnType(block.m_type, &block.m_bits);
			}

			//make the block stone if it is lower than 7 below the half point line of the chunk
			else
			{
				BlockDefinition* blockDefinitionForType = BlockDefinition::GetDefinitionById(STONE_BLOCK_ID);
				m_blocks[GetBlockIndexForBlockCoords(IntVector3(columnCoordinates.x, columnCoordinates.y, blockIndexInColumn))].m_type = blockDefinitionForType->m_type;
				m_blocks[GetBlockIndexForBlockCoords(IntVector3(columnCoordinates.x, columnCoordinates.y, blockIndexInColumn))].m_bits = blockDefinitionForType->m_defaultBits;
				//BlockDefinition::UpdateBitsBasedOnType(block.m_type, &block.m_bits);
			}			

			
		}
	}
}

//  =========================================================================================
void Chunk::GenerateChunkMesh()
{
	m_meshBuilder->FlushBuilder();

	for (int blockIndex = 0; blockIndex < BLOCKS_PER_CHUNK; ++blockIndex)
	{
		AddBlockToMesh(blockIndex, GetBlockWorldCenterForBlockIndex(blockIndex), &m_blocks[blockIndex]);
	}

	m_gpuMesh = m_meshBuilder->CreateMesh<VertexPCU>();
}

//  =========================================================================================
Rgba Chunk::GetVertexColorFromBlockLightingValue(Block* block)
{
	Rgba vertexColor = g_maxLightColor;
	uint8 highestLightingValue = 0;
	uint8 indoorLightingValue = block->GetIndoorLightingValue();
	uint8 outdoorLightingValue = block->GetOutDoorLightingValue();
	
	/*highestLightingValue = indoorLightingValue;

	if (indoorLightingValue < outdoorLightingValue)
	{
		highestLightingValue = outdoorLightingValue;
	}

	float percentageExposure = RangeMapFloat((float)highestLightingValue, 0.f, 15.f, MIN_LIGHT_EXPOSURE_PERCENTAGE, MAX_LIGHT_EXPOSURE_PERCENTAGE);*/

	
	//ClampFloat(ONE_SIXTEENTH * (float)(2 + indoorLightingValue), 0.f, 1.f)
	vertexColor.SetRedAsFloat(ONE_SIXTEENTH * (float)(1 + indoorLightingValue));
	vertexColor.SetGreenAsFloat(ONE_FIFTEENTH * (float)(outdoorLightingValue));

	//vertexColor.ScaleRGBByPercentage(percentageExposure);
	return vertexColor;
}

//  =========================================================================================
int Chunk::GetBlockIndexForBlockCoords(const IntVector3& blockCoords)
{
	// FAST VERSION return blockCoords.x | (blockCoords.y << CHUNK_X_MASK) + (blockCoords.z << (CHUNK_X_MASK + CHUNK_Y_MASK));
	return blockCoords.x + (blockCoords.y * BLOCKS_WIDE_X) + (blockCoords.z * (BLOCKS_WIDE_X * BLOCKS_WIDE_Y));
}

//  =========================================================================================
IntVector3 Chunk::GetBlockCoordsForBlockIndex(int blockIndex)
{
	int x = blockIndex & CHUNK_X_MASK;
	int y = (blockIndex & CHUNK_Y_MASK) >> BITS_WIDE_X;
	int z = blockIndex >> (BITS_WIDE_X + BITS_WIDE_Y);	

	return IntVector3(x, y, z);
}

//  =========================================================================================
Vector3 Chunk::GetBlockWorldCoordsForBlockIndex(int blockIndex)
{
	Vector3 worldCoordinates = Vector3(GetBlockCoordsForBlockIndex(blockIndex));
	return worldCoordinates + m_worldBounds.mins;
}

//  =========================================================================================
Vector3 Chunk::GetBlockWorldCenterForBlockIndex(int blockIndex)
{
	return GetBlockWorldCoordsForBlockIndex(blockIndex) + blockCenterOffset;
}

//  =========================================================================================
bool Chunk::GetBlockIndexForWorldPositionWithinBounds(uint& blockIndexOut, const Vector3& worldPosition)
{
	if(!m_worldBounds.IsPointInside(worldPosition))
		return false;

	Vector3 relativePosition = Vector3(worldPosition.x - m_worldBounds.mins.x, worldPosition.y - m_worldBounds.mins.y, worldPosition.z);
	IntVector3 relativeCoords = IntVector3((int)floorf(relativePosition.x), (int)floorf(relativePosition.y), (int)floorf(relativePosition.z));

	blockIndexOut = GetBlockIndexForBlockCoords(relativeCoords);

	return true;
}

//  =========================================================================================
void Chunk::AddBlockToMesh(const int blockIndex, const Vector3& center, Block* block)
{
	if(!block->IsVisible())
		return;

	float xVal = 0.5f;
	float yVal = 0.5f;
	float zVal = 0.5f;

	BlockDefinition* blockDef = BlockDefinition::GetDefinitionById(block->m_type);

	AABB2 frontTexCoords = GetTerrainSprites()->GetTexCoordsForSpriteCoords(blockDef->m_frontTexCoords);
	AABB2 rightTexCoords = GetTerrainSprites()->GetTexCoordsForSpriteCoords(blockDef->m_rightSideTexCoords);
	AABB2 backTexCoords = GetTerrainSprites()->GetTexCoordsForSpriteCoords(blockDef->m_backTexCoords);
	AABB2 leftTexCoords = GetTerrainSprites()->GetTexCoordsForSpriteCoords(blockDef->m_leftSideTexCoords);
	AABB2 topTexCoords = GetTerrainSprites()->GetTexCoordsForSpriteCoords(blockDef->m_topTexCoords);
	AABB2 bottomTexCoords = GetTerrainSprites()->GetTexCoordsForSpriteCoords(blockDef->m_bottomTexCoords);

	Rgba tint = Rgba::WHITE;

	int vertSize = m_meshBuilder->GetVertexCount();

	m_meshBuilder->Begin(TRIANGLES_DRAW_PRIMITIVE, true); //begin is does use index buffer

	//ASSERT_OR_DIE(DoesHaveAllNeighbors(), "SOME NEIGHBORS NOT FOUND WHEN TRYING TO BUILD MESH!!");

	//get block locators for neighboring blocks
	BlockLocator blockLocator = BlockLocator(this, blockIndex);
	BlockLocator westLocator = blockLocator.GetBlockLocatorToWest();
	BlockLocator southLocator = blockLocator.GetBlockLocatorToSouth();
	BlockLocator eastLocator = blockLocator.GetBlockLocatorToEast();
	BlockLocator northLocator = blockLocator.GetBlockLocatorToNorth();	
	BlockLocator aboveLocator = blockLocator.GetBlockLocatorAbove();
	BlockLocator bottomLocator = blockLocator.GetBlockLocatorBelow();

	//west/front face
	if (!westLocator.GetBlock()->IsVisible())
	{
		Rgba westTint = GetVertexColorFromBlockLightingValue(westLocator.GetBlock());

		m_meshBuilder->SetColor(westTint);
		m_meshBuilder->SetUV(frontTexCoords.maxs.x, frontTexCoords.maxs.y);
		m_meshBuilder->PushVertex(Vector3(center.x - xVal, center.y + yVal, center.z - zVal));

		m_meshBuilder->SetColor(westTint);
		m_meshBuilder->SetUV(frontTexCoords.mins.x, frontTexCoords.maxs.y);	
		m_meshBuilder->PushVertex(Vector3(center.x - xVal, center.y - yVal, center.z - zVal));

		m_meshBuilder->SetColor(westTint);
		m_meshBuilder->SetUV(frontTexCoords.mins.x, frontTexCoords.mins.y);
		m_meshBuilder->PushVertex(Vector3(center.x - xVal, center.y - yVal, center.z + zVal));

		m_meshBuilder->SetColor(westTint);
		m_meshBuilder->SetUV(frontTexCoords.maxs.x, frontTexCoords.mins.y);
		m_meshBuilder->PushVertex(Vector3(center.x - xVal, center.y + yVal, center.z + zVal));

		m_meshBuilder->AddQuadIndices(vertSize, vertSize + 1, vertSize + 2, vertSize + 3);
		vertSize += 4;
	}

	//south/right face
	if (!southLocator.GetBlock()->IsVisible())
	{
		Rgba southTint = GetVertexColorFromBlockLightingValue(southLocator.GetBlock());

		m_meshBuilder->SetColor(southTint);
		m_meshBuilder->SetUV(rightTexCoords.maxs.x, rightTexCoords.maxs.y);
		m_meshBuilder->PushVertex(Vector3(center.x - xVal, center.y - yVal, center.z - zVal));

		m_meshBuilder->SetColor(southTint);
		m_meshBuilder->SetUV(rightTexCoords.mins.x, rightTexCoords.maxs.y);
		m_meshBuilder->PushVertex(Vector3(center.x + xVal, center.y - yVal, center.z - zVal));

		m_meshBuilder->SetColor(southTint);
		m_meshBuilder->SetUV(rightTexCoords.mins.x, rightTexCoords.mins.y);
		m_meshBuilder->PushVertex(Vector3(center.x + xVal, center.y - yVal, center.z + zVal));

		m_meshBuilder->SetColor(southTint);
		m_meshBuilder->SetUV(rightTexCoords.maxs.x, rightTexCoords.mins.y);
		m_meshBuilder->PushVertex(Vector3(center.x - xVal, center.y - yVal, center.z + zVal));

		m_meshBuilder->AddQuadIndices(vertSize, vertSize + 1, vertSize + 2, vertSize + 3);
		vertSize += 4;
	}

	//east face
	//back face
	if (!eastLocator.GetBlock()->IsVisible())
	{
		Rgba eastTint = GetVertexColorFromBlockLightingValue(eastLocator.GetBlock());

		m_meshBuilder->SetColor(eastTint);
		m_meshBuilder->SetUV(backTexCoords.maxs.x, backTexCoords.maxs.y);
		m_meshBuilder->PushVertex(Vector3(center.x + xVal, center.y - yVal, center.z - zVal));

		m_meshBuilder->SetColor(eastTint);
		m_meshBuilder->SetUV(backTexCoords.mins.x, backTexCoords.maxs.y);
		m_meshBuilder->PushVertex(Vector3(center.x + xVal, center.y + yVal, center.z - zVal));

		m_meshBuilder->SetColor(eastTint);
		m_meshBuilder->SetUV(backTexCoords.mins.x, backTexCoords.mins.y);
		m_meshBuilder->PushVertex(Vector3(center.x + xVal, center.y + yVal, center.z + zVal));

		m_meshBuilder->SetColor(eastTint);
		m_meshBuilder->SetUV(backTexCoords.maxs.x, backTexCoords.mins.y);
		m_meshBuilder->PushVertex(Vector3(center.x + xVal, center.y - yVal, center.z + zVal));

		m_meshBuilder->AddQuadIndices(vertSize, vertSize + 1, vertSize + 2, vertSize + 3);
		vertSize += 4;
	}

	//north face
	//left face
	if (!northLocator.GetBlock()->IsVisible())
	{
		Rgba northTint = GetVertexColorFromBlockLightingValue(northLocator.GetBlock());

		m_meshBuilder->SetColor(northTint);
		m_meshBuilder->SetUV(leftTexCoords.maxs.x, leftTexCoords.maxs.y);
		m_meshBuilder->PushVertex(Vector3(center.x + xVal, center.y + yVal, center.z - zVal));

		m_meshBuilder->SetColor(northTint);
		m_meshBuilder->SetUV(leftTexCoords.mins.x, leftTexCoords.maxs.y);
		m_meshBuilder->PushVertex(Vector3(center.x - xVal, center.y + yVal, center.z - zVal));

		m_meshBuilder->SetColor(northTint);	
		m_meshBuilder->SetUV(leftTexCoords.mins.x, leftTexCoords.mins.y);
		m_meshBuilder->PushVertex(Vector3(center.x - xVal, center.y + yVal, center.z + zVal));

		m_meshBuilder->SetColor(northTint);
		m_meshBuilder->SetUV(leftTexCoords.maxs.x, leftTexCoords.mins.y);
		m_meshBuilder->PushVertex(Vector3(center.x + xVal, center.y + yVal, center.z + zVal));

		m_meshBuilder->AddQuadIndices(vertSize, vertSize + 1, vertSize + 2, vertSize + 3);
		vertSize += 4;
	}

	//up face
	//top face
	if (aboveLocator.IsValid())
	{
		if (!aboveLocator.GetBlock()->IsVisible())
		{
			Rgba topTint = GetVertexColorFromBlockLightingValue(aboveLocator.GetBlock());

			m_meshBuilder->SetColor(topTint);
			m_meshBuilder->SetUV(topTexCoords.maxs.x, topTexCoords.maxs.y);
			m_meshBuilder->PushVertex(Vector3(center.x - xVal, center.y + yVal, center.z + zVal));

			m_meshBuilder->SetColor(topTint);
			m_meshBuilder->SetUV(topTexCoords.mins.x, topTexCoords.maxs.y);
			m_meshBuilder->PushVertex(Vector3(center.x - xVal, center.y - yVal, center.z + zVal));

			m_meshBuilder->SetColor(topTint);	
			m_meshBuilder->SetUV(topTexCoords.mins.x, topTexCoords.mins.y);
			m_meshBuilder->PushVertex(Vector3(center.x + xVal, center.y - yVal, center.z + zVal));

			m_meshBuilder->SetColor(topTint);
			m_meshBuilder->SetUV(topTexCoords.maxs.x, topTexCoords.mins.y);
			m_meshBuilder->PushVertex(Vector3(center.x + xVal, center.y + yVal, center.z + zVal));

			m_meshBuilder->AddQuadIndices(vertSize, vertSize + 1, vertSize + 2, vertSize + 3);
			vertSize += 4;
		}
	}

	//down face
	//bottom face
	if (bottomLocator.IsValid())
	{
		if (!bottomLocator.GetBlock()->IsVisible())
		{
			Rgba bottomTint = GetVertexColorFromBlockLightingValue(bottomLocator.GetBlock());

			m_meshBuilder->SetColor(bottomTint);
			m_meshBuilder->SetUV(bottomTexCoords.maxs.x, bottomTexCoords.maxs.y);
			m_meshBuilder->PushVertex(Vector3(center.x + xVal, center.y + yVal, center.z - zVal));

			m_meshBuilder->SetColor(bottomTint);
			m_meshBuilder->SetUV(bottomTexCoords.mins.x, bottomTexCoords.maxs.y);
			m_meshBuilder->PushVertex(Vector3(center.x + xVal, center.y - yVal, center.z - zVal));

			m_meshBuilder->SetColor(bottomTint);	
			m_meshBuilder->SetUV(bottomTexCoords.mins.x, bottomTexCoords.mins.y);
			m_meshBuilder->PushVertex(Vector3(center.x - xVal, center.y - yVal, center.z - zVal));

			m_meshBuilder->SetColor(bottomTint);	
			m_meshBuilder->SetUV(bottomTexCoords.maxs.x, bottomTexCoords.mins.y);
			m_meshBuilder->PushVertex(Vector3(center.x - xVal, center.y + yVal, center.z - zVal));

			m_meshBuilder->AddQuadIndices(vertSize, vertSize + 1, vertSize + 2, vertSize + 3);
			vertSize += 4;
		}
	}
}

//  =========================================================================================
void Chunk::AddNeighbor(Chunk* neighbor, eNeighborType neighborDirection)
{
	switch (neighborDirection)
	{
	case NORTH_NEIGHBOR_TYPE:
		m_northNeighbor = neighbor;

		//prevents infinite loop
		if(neighbor->m_southNeighbor == this)
			break;

		neighbor->AddNeighbor(this, SOUTH_NEIGHBOR_TYPE);
		break;
	case WEST_NEIGHBOR_TYPE:
		m_westNeighbor = neighbor;

		//prevents infinite loop
		if(neighbor->m_eastNeighbor == this)
			break;

		neighbor->AddNeighbor(this, EAST_NEIGHBOR_TYPE);
		break;
	case SOUTH_NEIGHBOR_TYPE:
		m_southNeighbor = neighbor;

		//prevents infinite loop
		if(neighbor->m_northNeighbor == this)
			break;

		neighbor->AddNeighbor(this, NORTH_NEIGHBOR_TYPE);
		break;
	case EAST_NEIGHBOR_TYPE:
		m_eastNeighbor = neighbor;

		//prevents infinite loop
		if(neighbor->m_westNeighbor == this)
			break;

		neighbor->AddNeighbor(this, WEST_NEIGHBOR_TYPE);
		break;
	}
}

//  =========================================================================================
void Chunk::RemoveNeighbor(eNeighborType neighborDirection)
{
	switch (neighborDirection)
	{
	case NORTH_NEIGHBOR_TYPE:
		//if (m_northNeighbor->m_southNeighbor != nullptr)
		//{
		//	m_northNeighbor->RemoveNeighbor(SOUTH_NEIGHBOR_TYPE);
		//}		
		//m_northNeighbor = nullptr;
		if(m_northNeighbor == nullptr)
			break;

		m_northNeighbor->m_southNeighbor = nullptr;
		m_northNeighbor = nullptr;
		break;
	case SOUTH_NEIGHBOR_TYPE:
		//if (m_southNeighbor->m_northNeighbor != nullptr)
		//{
		//	m_southNeighbor->RemoveNeighbor(NORTH_NEIGHBOR_TYPE);
		//}
		//m_southNeighbor = nullptr;
		if(m_southNeighbor == nullptr)
			break;

		m_southNeighbor->m_northNeighbor = nullptr;
		m_southNeighbor = nullptr;
		break;
	case EAST_NEIGHBOR_TYPE:
		//if (m_eastNeighbor->m_westNeighbor != nullptr)
		//{
		//	m_eastNeighbor->RemoveNeighbor(WEST_NEIGHBOR_TYPE);
		//}
		//m_eastNeighbor = nullptr;
		if(m_eastNeighbor == nullptr)
			break;

		m_eastNeighbor->m_westNeighbor = nullptr;
		m_eastNeighbor = nullptr;
		break;
	case WEST_NEIGHBOR_TYPE:
	/*	if (m_westNeighbor->m_eastNeighbor != nullptr)
		{
			m_westNeighbor->RemoveNeighbor(EAST_NEIGHBOR_TYPE);
		}
		m_westNeighbor = nullptr;
		*/
		if(m_westNeighbor == nullptr)
			break;

		m_westNeighbor->m_eastNeighbor = nullptr;
		m_westNeighbor = nullptr;
		break;
	}
}

//  =========================================================================================
bool Chunk::DoesHaveAllNeighbors()
{
	if (m_northNeighbor == nullptr ||
		m_westNeighbor == nullptr ||
		m_southNeighbor == nullptr ||
		m_eastNeighbor == nullptr)
	{
		return false;
	}

	return true;
}

//  =========================================================================================
void Chunk::UnhookNeighbors()
{
	RemoveNeighbor(NORTH_NEIGHBOR_TYPE);
	RemoveNeighbor(SOUTH_NEIGHBOR_TYPE);
	RemoveNeighbor(EAST_NEIGHBOR_TYPE);
	RemoveNeighbor(WEST_NEIGHBOR_TYPE);
}

//  =========================================================================================
//Vector3 Chunk::GetWorldPositionOfBlockZero()
//{
//	return Vector3((float)m_chunkCoords.x * BLOCKS_WIDE_X, (float)m_chunkCoords.y * BLOCKS_WIDE_Y, 0.f);
//}
