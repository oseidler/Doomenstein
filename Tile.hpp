#pragma once
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Vertex_PNCU.hpp"
#include "Engine/Math/AABB2.hpp"


//forward declarations
class TileDefinition;
class SpriteSheet;


class Tile
{
//public member functions
public:
	//constructors
	Tile() = default;
	Tile(IntVec2 tileCoords, TileDefinition const* definition);

	//tile utilities
	void AddVertsForTile(std::vector<Vertex_PNCU>& verts, std::vector<unsigned int>& indexes, SpriteSheet const* spriteSheet, IntVec2 const spriteSheetDimensions);

	//accessors
	AABB2 GetTileAABB2() const;

//public member variables
public:
	IntVec2 m_coords = IntVec2(-1, -1);
	TileDefinition const* m_definition;
};
