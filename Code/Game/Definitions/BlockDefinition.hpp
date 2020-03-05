#pragma once
#include "Engine\Core\XMLUtilities.hpp"
#include "Engine\ThirdParty\tinyxml2\tinyxml2.h"
#include "Game\GameCommon.hpp"
#include <string>
#include <map>
#include <vector>

class BlockDefinition
{
public:
	explicit BlockDefinition(const tinyxml2::XMLElement& element);
	static void Initialize(const std::string& filePath);
	static BlockDefinition* GetDefinitionById(const uint8 id);

private:

	static void SetGlobalBlockIdNameVariables();
	void ConstructBitsFromBools();

public:
	std::string m_name = "";
	uint8 m_type = 0;
	uint8 m_defaultBits = 0;

	bool m_isFullOpaque = false;
	bool m_isSolid = false;
	bool m_isVisible = false;
	bool m_doesEmitLight = false;
	/*
	the other flags are set dynamically
	*/

	//texture coordinates for block
	IntVector2 m_frontTexCoords = IntVector2::ZERO;
	IntVector2 m_rightSideTexCoords = IntVector2::ZERO;
	IntVector2 m_backTexCoords = IntVector2::ZERO;
	IntVector2 m_leftSideTexCoords = IntVector2::ZERO;
	IntVector2 m_bottomTexCoords = IntVector2::ZERO;
	IntVector2 m_topTexCoords = IntVector2::ZERO;

	bool m_doesTarget = false;
	uint8 m_minimumLightingValue = 0;

	static std::map<uint8, BlockDefinition*> s_blockDefinitions;
};

//block data - 255 potential
extern uint8 AIR_BLOCK_ID;
extern uint8 GRASS_BLOCK_ID;
extern uint8 STONE_BLOCK_ID;
extern uint8 DIRT_BLOCK_ID;
extern uint8 GLOWSTONE_BLOCK_ID;
