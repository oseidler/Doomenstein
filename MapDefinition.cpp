#include "Game/MapDefinition.hpp"
#include "Game/Game.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Renderer/Texture.hpp"


//static variable declaration
std::vector<MapDefinition> MapDefinition::s_mapDefinitions;


//
//constructor
//
MapDefinition::MapDefinition(XmlElement const& element)
{
	m_name = ParseXmlAttribute(element, "name", m_name);

	std::string imageFilePath = ParseXmlAttribute(element, "image", "invalid image file path");
	m_image = Image(imageFilePath.c_str());

	std::string shaderFilePath = ParseXmlAttribute(element, "shader", "Data/Shaders/Default");
	m_shader = g_theRenderer->CreateShader(shaderFilePath.c_str());

	std::string spriteSheetFilePath = ParseXmlAttribute(element, "spriteSheetTexture", "invalid sprite sheet file path");
	m_spriteSheetTexture = g_theRenderer->CreateOrGetTextureFromFile(spriteSheetFilePath.c_str());
	m_spriteSheetCellCount = ParseXmlAttribute(element, "spriteSheetCellCount", m_spriteSheetCellCount);

	XmlElement const* spawnInfoRootElement = element.FirstChildElement();
	GUARANTEE_OR_DIE(spawnInfoRootElement != nullptr, "Failed to read spawn definitions element!");

	XmlElement const* spawnInfoElement = spawnInfoRootElement->FirstChildElement();
	while (spawnInfoElement != nullptr)
	{
		std::string elementName = spawnInfoElement->Name();
		GUARANTEE_OR_DIE(elementName == "SpawnInfo", "Spawn info elements in map definitions xml file must be <SpawnInfo>!");

		SpawnInfo spawnInfo;
		spawnInfo.m_actorName = ParseXmlAttribute(*spawnInfoElement, "actor", spawnInfo.m_actorName);
		spawnInfo.m_position = ParseXmlAttribute(*spawnInfoElement, "position", spawnInfo.m_position);
		spawnInfo.m_orientation = ParseXmlAttribute(*spawnInfoElement, "orientation", spawnInfo.m_orientation);
		spawnInfo.m_velocity = ParseXmlAttribute(*spawnInfoElement, "velocity", spawnInfo.m_velocity);
		m_spawnInfos.push_back(spawnInfo);
		spawnInfoElement = spawnInfoElement->NextSiblingElement();
	}
}


//
//static functions
//
void MapDefinition::InitializeMapDefs()
{
	XmlDocument mapDefsXml;
	char const* filePath = "Data/Definitions/MapDefinitions.xml";
	XmlError result = mapDefsXml.LoadFile(filePath);
	GUARANTEE_OR_DIE(result == tinyxml2::XML_SUCCESS, "Failed to open map definitions xml file!");

	XmlElement* rootElement = mapDefsXml.RootElement();
	GUARANTEE_OR_DIE(rootElement != nullptr, "Failed to read map definitions root element!");

	XmlElement* mapDefElement = rootElement->FirstChildElement();
	while (mapDefElement != nullptr)
	{
		std::string elementName = mapDefElement->Name();
		GUARANTEE_OR_DIE(elementName == "MapDefinition", "Child element names in map definitions xml file must be <MapDefinition>!");
		MapDefinition newMapDef = MapDefinition(*mapDefElement);
		s_mapDefinitions.push_back(newMapDef);
		mapDefElement = mapDefElement->NextSiblingElement();
	}
}


MapDefinition const* MapDefinition::GetMapDefinition(std::string name)
{
	for (int defIndex = 0; defIndex < s_mapDefinitions.size(); defIndex++)
	{
		if (s_mapDefinitions[defIndex].m_name == name)
		{
			return &s_mapDefinitions[defIndex];
		}
	}

	//return null if it wasn't found
	return nullptr;
}
