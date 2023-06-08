#include "Game/Tile.hpp"
#include "Game/TileDefinition.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"


//
//constructor
//
Tile::Tile(IntVec2 tileCoords, TileDefinition const* definition)
	: m_coords(tileCoords)
	, m_definition(definition)
{
}


//
//public tile utilities
//
void Tile::AddVertsForTile(std::vector<Vertex_PNCU>& verts, std::vector<unsigned int>& indexes, SpriteSheet const* spriteSheet, IntVec2 const spriteSheetDimensions)
{
	float tileMinX = static_cast<float>(m_coords.x);
	float tileMinY = static_cast<float>(m_coords.y);
	float tileMinZ = 0.0f;
	float tileMaxX = tileMinX + 1.0f;
	float tileMaxY = tileMinY + 1.0f;
	float tileMaxZ = 1.0f;

	Vec3 backBottomLeft = Vec3(tileMinX, tileMaxY, tileMinZ);
	Vec3 backBottomRight = Vec3(tileMinX, tileMinY, tileMinZ);
	Vec3 forwardBottomLeft = Vec3(tileMaxX, tileMaxY, tileMinZ);
	Vec3 forwardBottomRight = Vec3(tileMaxX, tileMinY, tileMinZ);

	Vec3 backTopLeft = Vec3(tileMinX, tileMaxY, tileMaxZ);
	Vec3 backTopRight = Vec3(tileMinX, tileMinY, tileMaxZ);
	Vec3 forwardTopLeft = Vec3(tileMaxX, tileMaxY, tileMaxZ);
	Vec3 forwardTopRight = Vec3(tileMaxX, tileMinY, tileMaxZ);

	//add verts for floor quad
	if (m_definition->m_floorSpriteCoords != IntVec2(-1, -1))
	{
		int spriteIndex = m_definition->m_floorSpriteCoords.x + m_definition->m_floorSpriteCoords.y * spriteSheetDimensions.x;
		AABB2 uvs = spriteSheet->GetSpriteUVs(spriteIndex);

		AddVertsForQuad3D(verts, indexes, backBottomLeft, backBottomRight, forwardBottomLeft, forwardBottomRight, Rgba8(), uvs);	//TO-DO: Add UVs
	}

	//add verts for four wall quads
	if (m_definition->m_wallSpriteCoords != IntVec2(-1, -1))
	{
		int spriteIndex = m_definition->m_wallSpriteCoords.x + m_definition->m_wallSpriteCoords.y * spriteSheetDimensions.x;
		AABB2 uvs = spriteSheet->GetSpriteUVs(spriteIndex);

		AddVertsForQuad3D(verts, indexes, backBottomRight, forwardBottomRight, backTopRight, forwardTopRight, Rgba8(), uvs);
		AddVertsForQuad3D(verts, indexes, backBottomLeft, backBottomRight, backTopLeft, backTopRight, Rgba8(), uvs);
		AddVertsForQuad3D(verts, indexes, forwardBottomLeft, backBottomLeft, forwardTopLeft, backTopLeft, Rgba8(), uvs);
		AddVertsForQuad3D(verts, indexes, forwardBottomRight, forwardBottomLeft, forwardTopRight, forwardTopLeft, Rgba8(), uvs);
	}

	//add verts for ceiling quad
	if (m_definition->m_ceilingSpriteCoords != IntVec2(-1, -1))
	{
		int spriteIndex = m_definition->m_ceilingSpriteCoords.x + m_definition->m_ceilingSpriteCoords.y * spriteSheetDimensions.x;
		AABB2 uvs = spriteSheet->GetSpriteUVs(spriteIndex);

		AddVertsForQuad3D(verts, indexes, forwardTopLeft, forwardTopRight, backTopLeft, backTopRight, Rgba8(), uvs);
	}
}


//
//accessors
//
AABB2 Tile::GetTileAABB2() const
{
	return AABB2(static_cast<float>(m_coords.x), static_cast<float>(m_coords.y), static_cast<float>(m_coords.x + 1), static_cast<float>(m_coords.y + 1));
}
