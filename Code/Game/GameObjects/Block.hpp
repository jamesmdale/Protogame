#pragma once
#include "Engine\Math\IntVector2.hpp"
#include "Engine\Core\EngineCommon.hpp"
#include "Game\Definitions\BlockDefinition.hpp"
#include "Game\GameCommon.hpp"

class Block
{
public:
	Block();
	Block(BlockDefinition* definition);
	~Block();

	inline bool IsValid();

	//helpers for bitfields
	inline bool IsAir();
	inline bool IsFullOpaque();
	inline bool IsVisible();
	inline bool IsSolid();
	inline bool DoesEmitLight();

	//dynamic bitfields
	inline bool IsSky();
	inline bool IsLightingInDirtyList();

	//helpers for lighting
	inline void SetIndoorLightingValue(const uint8 lightingValue);
	inline void SetOutdoorLightingValue(const uint8 lightingValue);
	inline uint8 GetIndoorLightingValue();
	inline uint8 GetOutDoorLightingValue();

	//setters for bit fields
	inline void SetSkyFlag(bool isSky);
	inline void SetLightingInDirtyListFlag(bool isLightingInDirtyList); 

public:
	uint8 m_type = UINT8_MAX; //max 255 types
	uint8 m_bits = 0; //state
	uint8 m_lighting = 0; //lighting value
};


extern Block* g_invalidBlock;

// global statics =========================================================================================
void SetBlockToType(Block* block, const uint8 id);

//  =========================================================================================
//  INLINE FUNCTIONS
//  =========================================================================================
bool Block::IsValid()
{
	if(m_type != UINT8_MAX)
		return true;

	return false;
}

//  =========================================================================================
bool Block::IsAir()
{
	if(m_type == AIR_BLOCK_ID)
		return true;

	return false;
}

//  =========================================================================================
bool Block::IsFullOpaque()
{
	return (m_bits & IS_BLOCK_FULL_OPAQUE_MASK) == IS_BLOCK_FULL_OPAQUE_MASK;
}

//  =========================================================================================
bool Block::IsVisible()
{
	return (m_bits & IS_BLOCK_VISIBLE_MASK) == IS_BLOCK_VISIBLE_MASK;
}

//  =========================================================================================
bool Block::IsSolid()
{
	return (m_bits & IS_BLOCK_SOLID_MASK) == IS_BLOCK_SOLID_MASK;
}

bool Block::DoesEmitLight()
{
	return (m_bits & DOES_BLOCK_EMIT_LIGHT) == DOES_BLOCK_EMIT_LIGHT;
}

//  =========================================================================================
bool Block::IsSky()
{
	return (m_bits & IS_BLOCK_SKY_MASK) == IS_BLOCK_SKY_MASK;
}

//  =========================================================================================
bool Block::IsLightingInDirtyList()
{
	return (m_bits & IS_BLOCK_IN_LIGHT_DIRTY_LIST_MASK) == IS_BLOCK_IN_LIGHT_DIRTY_LIST_MASK;
}

//  =========================================================================================
void Block::SetSkyFlag(bool isSky)
{
	if (isSky == true)
	{
		m_bits |= IS_BLOCK_SKY_MASK;
	}
	else
	{
		m_bits &= (~IS_BLOCK_SKY_MASK);
	}
}

//  =========================================================================================
void Block::SetLightingInDirtyListFlag(bool isLightingDirty)
{
	if (isLightingDirty == true)
	{
		m_bits |= IS_BLOCK_IN_LIGHT_DIRTY_LIST_MASK;
	}
	else
	{
		m_bits &= (~IS_BLOCK_IN_LIGHT_DIRTY_LIST_MASK);
	}
}

//  =========================================================================================
void Block::SetIndoorLightingValue(const uint8 lightingValue)
{
	//make sure the lighting value isn't greater than 15
	uint8 lightingValueClamped = lightingValue & (~OUTDOOR_LIGHTING_MASK);

	m_lighting &= (~INDOOR_LIGHTING_MASK);
	m_lighting |= lightingValueClamped;
}

//  =========================================================================================
void Block::SetOutdoorLightingValue(const uint8 lightingValue)
{
	uint8 lightingValueShifted = lightingValue << BITS_WIDE_INDOOR_LIGHTING_MASK;

	m_lighting &= (~OUTDOOR_LIGHTING_MASK);
	m_lighting |= lightingValueShifted;
}

//  =========================================================================================
uint8 Block::GetIndoorLightingValue()
{
	uint8 indoorLightingValue = m_lighting & (~OUTDOOR_LIGHTING_MASK);
	return indoorLightingValue;
}

//  =========================================================================================
uint8 Block::GetOutDoorLightingValue()
{
	uint8 outDoorLightingValue = m_lighting >> BITS_WIDE_INDOOR_LIGHTING_MASK;
	return outDoorLightingValue;
}