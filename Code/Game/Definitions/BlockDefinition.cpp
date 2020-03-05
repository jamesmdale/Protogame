#include "Game\Definitions\BlockDefinition.hpp"
#include "Engine\Core\EngineCommon.hpp"
#include "Engine\Core\StringUtils.hpp"

std::map<uint8, BlockDefinition*> BlockDefinition::s_blockDefinitions;

//block types

#pragma region "Block IDs"
	uint8 AIR_BLOCK_ID = UINT8_MAX;
	uint8 GRASS_BLOCK_ID = UINT8_MAX;
	uint8 STONE_BLOCK_ID = UINT8_MAX;
	uint8 DIRT_BLOCK_ID = UINT8_MAX;
	uint8 GLOWSTONE_BLOCK_ID = UINT8_MAX;
#pragma endregion
	

//  =========================================================================================
BlockDefinition::BlockDefinition(const tinyxml2::XMLElement& element)
{
	m_type = ParseXmlAttribute(element, "id", m_type);
	m_name = ParseXmlAttribute(element, "name", m_name);

	//get default bit settings
	const tinyxml2::XMLElement* bitSettingsElement = element.FirstChildElement("BitSettings");
	if (bitSettingsElement)
	{
		m_isFullOpaque = ParseXmlAttribute(*bitSettingsElement, "isFullOpaque", m_isFullOpaque);
		m_isSolid = ParseXmlAttribute(*bitSettingsElement, "isSolid", m_isSolid);
		m_isVisible = ParseXmlAttribute(*bitSettingsElement, "isVisible", m_isVisible);
		m_doesEmitLight = ParseXmlAttribute(*bitSettingsElement, "doesEmitLight", m_doesEmitLight);

		//other bits are set dynamically
	}

	//lighting
	const tinyxml2::XMLElement* defaultLightSettingsElement = element.FirstChildElement("Lighting");
	if (defaultLightSettingsElement)
	{
		m_minimumLightingValue = ParseXmlAttribute(*defaultLightSettingsElement, "minimumLightingValue", m_minimumLightingValue);
	}

	//get image value
	const tinyxml2::XMLElement* texcoordElement = element.FirstChildElement("TextureCoords");
	if (texcoordElement)
	{
		m_frontTexCoords = ParseXmlAttribute(*texcoordElement, "front", m_frontTexCoords);
		m_rightSideTexCoords = ParseXmlAttribute(*texcoordElement, "right", m_rightSideTexCoords);
		m_backTexCoords = ParseXmlAttribute(*texcoordElement, "back", m_backTexCoords);
		m_leftSideTexCoords = ParseXmlAttribute(*texcoordElement, "left", m_leftSideTexCoords);
		m_topTexCoords = ParseXmlAttribute(*texcoordElement, "top", m_topTexCoords);	
		m_bottomTexCoords = ParseXmlAttribute(*texcoordElement, "bottom", m_bottomTexCoords);		
	}

	ConstructBitsFromBools();
}

//  =========================================================================================
void BlockDefinition::Initialize(const std::string& filePath)
{
	tinyxml2::XMLDocument tileDefDoc;
	tileDefDoc.LoadFile(filePath.c_str());

	tinyxml2::XMLElement* pRoot = tileDefDoc.FirstChildElement();

	for (const tinyxml2::XMLElement* definitionNode = pRoot->FirstChildElement(); definitionNode; definitionNode = definitionNode->NextSiblingElement())
	{
		BlockDefinition* newDef = new BlockDefinition(*definitionNode);
		s_blockDefinitions.insert(std::pair<uint8, BlockDefinition*>(newDef->m_type, newDef));
	}

	SetGlobalBlockIdNameVariables();
	BlockDefinition* definition = nullptr;
}

//  =========================================================================================
BlockDefinition* BlockDefinition::GetDefinitionById(const uint8 id)
{
	std::map<uint8, BlockDefinition*>::iterator mapIterator = s_blockDefinitions.find(id);
	if (mapIterator == s_blockDefinitions.end()) {
		return nullptr;
	}
	else 
	{
		return mapIterator->second;
	}
}

//  =========================================================================================
void BlockDefinition::ConstructBitsFromBools()
{
	if (m_isFullOpaque)
		m_defaultBits |= IS_BLOCK_FULL_OPAQUE_MASK;

	if(m_isVisible)
		m_defaultBits |= IS_BLOCK_VISIBLE_MASK;

	if(m_isSolid)
		m_defaultBits |= IS_BLOCK_SOLID_MASK;

	if(m_doesEmitLight)
		m_defaultBits |= DOES_BLOCK_EMIT_LIGHT;
}

//  =========================================================================================
void BlockDefinition::SetGlobalBlockIdNameVariables()
{
	std::map<uint8, BlockDefinition*>::iterator definitionIterator = s_blockDefinitions.begin();

	for (definitionIterator; definitionIterator != s_blockDefinitions.end(); ++definitionIterator)
	{
		BlockDefinition* def = definitionIterator->second;
		if (def->m_name == "air")
		{
			AIR_BLOCK_ID = def->m_type;
			continue;
		}

		if (def->m_name == "grass")
		{
			GRASS_BLOCK_ID = def->m_type;
			continue;
		}

		if (def->m_name == "dirt")
		{
			DIRT_BLOCK_ID = def->m_type;
			continue;
		}

		if (def->m_name == "stone")
		{
			STONE_BLOCK_ID = def->m_type;
			continue;
		}

		if (def->m_name == "glowstone")
		{
			GLOWSTONE_BLOCK_ID = def->m_type;
			continue;
		}
	}
}


