#include "Game\GameObjects\BlockLocator.hpp"
#include "Game\GameObjects\Chunk.hpp"
#include "Game\GameObjects\Block.hpp"
#include "Game\GameCommon.hpp"


//  =========================================================================================
BlockLocator::BlockLocator()
{

}

//  =========================================================================================
BlockLocator::BlockLocator(Chunk* chunk, int blockIndex)
{
	m_chunk = chunk;
	m_blockIndex = blockIndex;
}

//  =========================================================================================
BlockLocator::~BlockLocator()
{

}

//  =========================================================================================
bool BlockLocator::IsValid()
{
	//uint8 m_type = GetBlock()->m_type;
	if (m_chunk == nullptr || m_blockIndex == -1)
	{
		return false;
	}

	return true;
}

//  =========================================================================================
bool BlockLocator::IsBlockIndexOnEdge(std::vector<Chunk*>& outConnectingChunks)
{
	bool isBlockOnEdge = false;

	BlockLocator northLocator = GetBlockLocatorToNorth();
	BlockLocator southLocator = GetBlockLocatorToSouth();
	BlockLocator eastLocator = GetBlockLocatorToEast();
	BlockLocator westLocator = GetBlockLocatorToWest();

	if (northLocator.IsValid())
	{
		if (northLocator.m_chunk != m_chunk)
		{
			outConnectingChunks.push_back(northLocator.m_chunk);
			isBlockOnEdge = true;
		}
	}
	
	if (southLocator.IsValid())
	{
		if (southLocator.m_chunk != m_chunk)
		{
			outConnectingChunks.push_back(southLocator.m_chunk);
			isBlockOnEdge = true;
		}
	}

	if (eastLocator.IsValid())
	{
		if (eastLocator.m_chunk != m_chunk)
		{
			outConnectingChunks.push_back(eastLocator.m_chunk);
			isBlockOnEdge = true;
		}
	}

	if (westLocator.IsValid())
	{
		if (westLocator.m_chunk != m_chunk)
		{
			outConnectingChunks.push_back(westLocator.m_chunk);
			isBlockOnEdge = true;
		}
	}

	return isBlockOnEdge;
}

//  =========================================================================================
bool BlockLocator::IsBlockIndexOnEdgeFast()
{
	bool isBlockOnEdge = false;

	BlockLocator northLocator = GetBlockLocatorToNorth();
	if (northLocator.m_chunk != m_chunk)
	{
		isBlockOnEdge = true;
		return isBlockOnEdge;
	}

	BlockLocator southLocator = GetBlockLocatorToSouth();
	if (southLocator.m_chunk != m_chunk)
	{
		isBlockOnEdge = true;
		return isBlockOnEdge;
	}

	BlockLocator eastLocator = GetBlockLocatorToEast();
	if (eastLocator.m_chunk != m_chunk)
	{
		isBlockOnEdge = true;
		return isBlockOnEdge;
	}

	BlockLocator westLocator = GetBlockLocatorToWest();
	if (westLocator.m_chunk != m_chunk)
	{
		isBlockOnEdge = true;
		return isBlockOnEdge;
	}	

	return isBlockOnEdge;
}

//  =========================================================================================
Block* BlockLocator::GetBlock()
{
	if(m_chunk == nullptr)
		return g_invalidBlock;

	return &m_chunk->m_blocks[m_blockIndex];
}

//  =========================================================================================
void BlockLocator::StepNorth()
{
	if (!IsValid())
		return;

	int yIndex = m_blockIndex & (CHUNK_Y_MASK);
	if (yIndex != CHUNK_Y_MASK)
	{
		m_blockIndex = (m_blockIndex + BLOCKS_WIDE_X);
		return;
	}

	//error case if north neighbor doesn't exist
	if (m_chunk == nullptr)
	{
		return;
	}
	else
	{
		m_chunk = m_chunk->m_northNeighbor;
		m_blockIndex = m_blockIndex & (~CHUNK_Y_MASK);
		return;
	}	
}

//  =========================================================================================
void BlockLocator::StepWest()
{
	if (!IsValid())
		return ;

	int xIndex = m_blockIndex & (CHUNK_X_MASK);
	if (xIndex != 0)
	{
		m_blockIndex = (m_blockIndex - 1);
		return;
	}

	//error case if west neighbor doesn't exist
	if (m_chunk->m_westNeighbor == nullptr)
	{
		m_chunk = nullptr;
		m_blockIndex = -1;
		return;
	}
	else
	{
		m_chunk = m_chunk->m_westNeighbor;
		m_blockIndex = m_blockIndex | CHUNK_X_MASK;
		return;
	}
}

//  =========================================================================================
void BlockLocator::StepSouth()
{
	if (!IsValid())
		return ;

	int yIndex = m_blockIndex & (CHUNK_Y_MASK);
	if (yIndex != 0)
	{
		m_blockIndex = m_blockIndex - BLOCKS_WIDE_X;
		return;
	}

	//error case if south neighbor doesn't exist
	if (m_chunk == nullptr)
	{
		return;
	}
	else
	{
		m_chunk = m_chunk->m_southNeighbor;
		m_blockIndex = m_blockIndex | CHUNK_Y_MASK;
	}
}

//  =========================================================================================
void BlockLocator::StepEast()
{
	if (!IsValid())
		return;

	int xIndex = m_blockIndex & (CHUNK_X_MASK);
	if (xIndex != CHUNK_X_MASK)
	{
		m_blockIndex = (m_blockIndex + 1);
		return;
	}

	//error case if east neighbor doesn't exist
	if (m_chunk->m_eastNeighbor == nullptr)
	{
		m_chunk = nullptr;
		m_blockIndex = -1;
		return;
	}
	else
	{
		m_chunk = m_chunk->m_eastNeighbor;
		m_blockIndex = m_blockIndex & (~ CHUNK_X_MASK);
		return;
	}
}

//  =========================================================================================
void BlockLocator::StepUp()
{
	if (!IsValid())
		return;

	int zIndex = m_blockIndex & (CHUNK_Z_MASK);
	if (zIndex != CHUNK_Z_MASK)
	{
		m_blockIndex = m_blockIndex + (BLOCKS_WIDE_X * BLOCKS_WIDE_Y);
		return;
	}

	m_chunk = nullptr;
	m_blockIndex = -1;
}

//  =========================================================================================
void BlockLocator::StepDown()
{
	if (!IsValid())
		return;

	int zIndex = m_blockIndex & (CHUNK_Z_MASK);
	if (zIndex != 0)
	{
		m_blockIndex = m_blockIndex - (BLOCKS_WIDE_X * BLOCKS_WIDE_Y);
		return;
	}

	m_chunk = nullptr;
	m_blockIndex = -1;
}

//  =========================================================================================
BlockLocator BlockLocator::GetBlockLocatorToNorth()
{
	if (!IsValid())
		return g_invalidBlockLocator;

	int yIndex = m_blockIndex & (CHUNK_Y_MASK);
	if (yIndex != CHUNK_Y_MASK)
	{
		return BlockLocator(m_chunk, (m_blockIndex + BLOCKS_WIDE_X));
	}

	//error case if north neighbor doesn't exist
	if (m_chunk == nullptr)
	{
		return BlockLocator(nullptr, -1);
	}
	else
	{
		return BlockLocator(m_chunk->m_northNeighbor, m_blockIndex & (~CHUNK_Y_MASK));
	}	
}

//  =========================================================================================
BlockLocator BlockLocator::GetBlockLocatorToSouth()
{
	if (!IsValid())
		return g_invalidBlockLocator;

	int yIndex = m_blockIndex & (CHUNK_Y_MASK);
	if (yIndex != 0)
	{
		return BlockLocator(m_chunk, (m_blockIndex - BLOCKS_WIDE_X));
	}

	//if we don't exist, return invalid blocklocator
	if (m_chunk == nullptr)
	{
		return BlockLocator(nullptr, -1);
	}
	else
	{
		return BlockLocator(m_chunk->m_southNeighbor,  m_blockIndex | CHUNK_Y_MASK);
	}
}

//  ========================================================================================= 
BlockLocator BlockLocator::GetBlockLocatorToWest()
{
	if (!IsValid())
		return g_invalidBlockLocator;

	int xIndex = m_blockIndex & (CHUNK_X_MASK);
	if (xIndex != 0)
	{
		return BlockLocator(m_chunk, (m_blockIndex - 1));
	}

	//error case if west neighbor doesn't exist
	if (m_chunk->m_westNeighbor == nullptr)
	{
		return BlockLocator(nullptr, -1);
	}
	else
	{
		return BlockLocator(m_chunk->m_westNeighbor, m_blockIndex | CHUNK_X_MASK);
	}
}


//  =========================================================================================
BlockLocator BlockLocator::GetBlockLocatorToEast()
{
	if (!IsValid())
		return g_invalidBlockLocator;

	int xIndex = m_blockIndex & (CHUNK_X_MASK);
	if (xIndex != CHUNK_X_MASK)
	{
		return BlockLocator(m_chunk, (m_blockIndex + 1));
	}

	//error case if east neighbor doesn't exist
	if (m_chunk->m_eastNeighbor == nullptr)
	{
		return BlockLocator(nullptr, -1);
	}
	else
	{
		return BlockLocator(m_chunk->m_eastNeighbor, m_blockIndex & (~ CHUNK_X_MASK));
	}
}

//  =========================================================================================
BlockLocator BlockLocator::GetBlockLocatorAbove()
{
	if (!IsValid())
		return g_invalidBlockLocator;

	int zIndex = m_blockIndex & (CHUNK_Z_MASK);
	if (zIndex != CHUNK_Z_MASK)
	{
		return BlockLocator(m_chunk, (m_blockIndex + (BLOCKS_WIDE_X * BLOCKS_WIDE_Y)));
	}

	return BlockLocator(nullptr, -1);
}

//  =========================================================================================
BlockLocator BlockLocator::GetBlockLocatorBelow()
{
	if (!IsValid())
		return g_invalidBlockLocator;

	int zIndex = m_blockIndex & (CHUNK_Z_MASK);
	if (zIndex != 0)
	{
		return BlockLocator(m_chunk, (m_blockIndex - (BLOCKS_WIDE_X * BLOCKS_WIDE_Y)));
	}

	return BlockLocator(nullptr, -1);
}

//  =========================================================================================
BlockLocator BlockLocator::GetBlockLocatorToNorthEast()
{
	if (!IsValid())
		return g_invalidBlockLocator;

	return GetBlockLocatorToNorth().GetBlockLocatorToEast();
}

//  =========================================================================================
BlockLocator BlockLocator::GetBlockLocatorToSouthEast()
{
	if (!IsValid())
		return g_invalidBlockLocator;

	return GetBlockLocatorToSouth().GetBlockLocatorToEast();
}

//  =========================================================================================
BlockLocator BlockLocator::GetBlockLocatorToNorthWest()
{
	if (!IsValid())
		return g_invalidBlockLocator;

	return GetBlockLocatorToNorth().GetBlockLocatorToWest();
}

//  =========================================================================================
BlockLocator BlockLocator::GetBlockLocatorToSouthWest()
{
	if (!IsValid())
		return g_invalidBlockLocator;

	return GetBlockLocatorToSouth().GetBlockLocatorToWest();
}

//  =========================================================================================
BlockLocator BlockLocator::GetBlockLocatorToAboveNorth()
{
	if (!IsValid())
		return g_invalidBlockLocator;

	return GetBlockLocatorAbove().GetBlockLocatorToNorth();
}

//  =========================================================================================
BlockLocator BlockLocator::GetBlockLocatorToAboveWest()
{
	if (!IsValid())
		return g_invalidBlockLocator;

	return GetBlockLocatorAbove().GetBlockLocatorToWest();
}

//  =========================================================================================
BlockLocator BlockLocator::GetBlockLocatorToAboveSouth()
{
	if (!IsValid())
		return g_invalidBlockLocator;

	return GetBlockLocatorAbove().GetBlockLocatorToSouth();
}

//  =========================================================================================
BlockLocator BlockLocator::GetBlockLocatorToAboveEast()
{
	if (!IsValid())
		return g_invalidBlockLocator;

	return GetBlockLocatorAbove().GetBlockLocatorToEast();
}

//  =========================================================================================
BlockLocator BlockLocator::GetBlockLocatorToBelowNorth()
{
	if (!IsValid())
		return g_invalidBlockLocator;

	return GetBlockLocatorBelow().GetBlockLocatorToNorth();
}

//  =========================================================================================
BlockLocator BlockLocator::GetBlockLocatorToBelowWest()
{
	if (!IsValid())
		return g_invalidBlockLocator;

	return GetBlockLocatorBelow().GetBlockLocatorToWest();
}

//  =========================================================================================
BlockLocator BlockLocator::GetBlockLocatorToBelowSouth()
{
	if (!IsValid())
		return g_invalidBlockLocator;

	return GetBlockLocatorBelow().GetBlockLocatorToSouth();
}

//  =========================================================================================
BlockLocator BlockLocator::GetBlockLocatorToBelowEast()
{
	if (!IsValid())
		return g_invalidBlockLocator;

	return GetBlockLocatorBelow().GetBlockLocatorToEast();
}

//  =========================================================================================
BlockLocator BlockLocator::GetBlockLocatorToAboveNorthEast()
{
	if (!IsValid())
		return g_invalidBlockLocator;

	return GetBlockLocatorAbove().GetBlockLocatorToNorth().GetBlockLocatorToEast();
}

//  =========================================================================================
BlockLocator BlockLocator::GetBlockLocatorToAboveNorthWest()
{
	if (!IsValid())
		return g_invalidBlockLocator;

	return GetBlockLocatorAbove().GetBlockLocatorToNorth().GetBlockLocatorToWest();
}

//  =========================================================================================
BlockLocator BlockLocator::GetBlockLocatorToAboveSouthEast()
{
	if (!IsValid())
		return g_invalidBlockLocator;

	return GetBlockLocatorAbove().GetBlockLocatorToSouth().GetBlockLocatorToEast();
}

//  =========================================================================================
BlockLocator BlockLocator::GetBlockLocatorToAboveSouthWest()
{
	if (!IsValid())
		return g_invalidBlockLocator;

	return GetBlockLocatorAbove().GetBlockLocatorToSouth().GetBlockLocatorToWest();
}

//  =========================================================================================
BlockLocator BlockLocator::GetBlockLocatorToBelowNorthEast()
{
	if (!IsValid())
		return g_invalidBlockLocator;

	return GetBlockLocatorBelow().GetBlockLocatorToNorth().GetBlockLocatorToEast();
}

//  =========================================================================================
BlockLocator BlockLocator::GetBlockLocatorToBelowNorthWest()
{
	if (!IsValid())
		return g_invalidBlockLocator;

	return GetBlockLocatorBelow().GetBlockLocatorToNorth().GetBlockLocatorToWest();
}

//  =========================================================================================
BlockLocator BlockLocator::GetBlockLocatorToBelowSouthEast()
{
	if (!IsValid())
		return g_invalidBlockLocator;

	return GetBlockLocatorBelow().GetBlockLocatorToSouth().GetBlockLocatorToEast();
}

//  =========================================================================================
BlockLocator BlockLocator::GetBlockLocatorToBelowSouthWest()
{
	if (!IsValid())
		return g_invalidBlockLocator;

	return GetBlockLocatorBelow().GetBlockLocatorToSouth().GetBlockLocatorToWest();
}
