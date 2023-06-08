#include "Game/TileDefinition.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"


//static variable declaration
std::vector<TileDefinition> TileDefinition::s_tileDefinitions;


//
//constructor
//
TileDefinition::TileDefinition(XmlElement const& element)
{
	m_name = ParseXmlAttribute(element, "name", m_name);
	m_isSolid = ParseXmlAttribute(element, "isSolid", false);
	m_mapImagePixelColor = ParseXmlAttribute(element, "mapImagePixelColor", m_mapImagePixelColor);
	m_floorSpriteCoords = ParseXmlAttribute(element, "floorSpriteCoords", m_floorSpriteCoords);
	m_wallSpriteCoords = ParseXmlAttribute(element, "wallSpriteCoords", m_wallSpriteCoords);
	m_ceilingSpriteCoords = ParseXmlAttribute(element, "ceilingSpriteCoords", m_ceilingSpriteCoords);
}


//static functions
void TileDefinition::InitializeTileDefs()
{
	XmlDocument tileDefsXml;
	char const* filePath = "Data/Definitions/TileDefinitions.xml";
	XmlError result = tileDefsXml.LoadFile(filePath);
	GUARANTEE_OR_DIE(result == tinyxml2::XML_SUCCESS, "Failed to open tile definitions xml file!");

	XmlElement* rootElement = tileDefsXml.RootElement();
	GUARANTEE_OR_DIE(rootElement != nullptr, "Failed to read tile definitions root element!");

	XmlElement* tileDefElement = rootElement->FirstChildElement();
	while (tileDefElement != nullptr)
	{
		std::string elementName = tileDefElement->Name();
		GUARANTEE_OR_DIE(elementName == "TileDefinition", "Child element names in tile definitions xml file must be <TileDefinition>!");
		TileDefinition newTileDef = TileDefinition(*tileDefElement);
		s_tileDefinitions.push_back(newTileDef);
		tileDefElement = tileDefElement->NextSiblingElement();
	}
}


TileDefinition const* TileDefinition::GetTileDefinition(std::string name)
{
	for (int defIndex = 0; defIndex < s_tileDefinitions.size(); defIndex++)
	{
		if (s_tileDefinitions[defIndex].m_name == name)
		{
			return &s_tileDefinitions[defIndex];
		}
	}

	//return null if it wasn't found
	return nullptr;
}
