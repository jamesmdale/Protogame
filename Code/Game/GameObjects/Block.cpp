#include "Game\GameObjects\Block.hpp"
#include "Game\Definitions\BlockDefinition.hpp"
#include "Game\GameCommon.hpp"

Block* g_invalidBlock = new Block();

//  =========================================================================================
Block::Block()
{

}

//  =========================================================================================
Block::Block(BlockDefinition* definition)
{
	m_type = definition->m_type;
	m_bits = definition->m_defaultBits;
	m_lighting = 0; //set all of our lighting bits off for now
}

//  =========================================================================================
Block::~Block()
{

}


//  =========================================================================================
void SetBlockToType(Block* block, const uint8 id)
{
	BlockDefinition* newDefinition = BlockDefinition::GetDefinitionById(id);

	block->m_type = newDefinition->m_type;
	block->m_bits = newDefinition->m_defaultBits;
	block->m_lighting = 0; //reset the lighting values
}
