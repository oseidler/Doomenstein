#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/IntVec2.hpp"


class TileDefinition
{
//public member variables
public:
	static std::vector<TileDefinition> s_tileDefinitions;

	//tile parameters
	std::string m_name = "invalid type";
	bool		m_isSolid = false;
	Rgba8		m_mapImagePixelColor = Rgba8(255, 255, 255);
	IntVec2		m_floorSpriteCoords = IntVec2(-1, -1);
	IntVec2		m_wallSpriteCoords = IntVec2(-1, -1);
	IntVec2		m_ceilingSpriteCoords = IntVec2(-1, -1);


//public member functions
public:
	//constructor
	explicit TileDefinition(XmlElement const& element);

	//static functions
	static void InitializeTileDefs();
	static TileDefinition const* GetTileDefinition(std::string name);
};
