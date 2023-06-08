#pragma once
#include "Game/SpawnInfo.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Core/Image.hpp"


//forward declarations
class Texture;
class Shader;


class MapDefinition
{
//public member variables
public:
	static std::vector<MapDefinition> s_mapDefinitions;

	//map parameters
	std::string m_name = "invalid type";
	Image		m_image = Image();
	Shader*		m_shader = nullptr;
	Texture*	m_spriteSheetTexture = nullptr;
	IntVec2		m_spriteSheetCellCount = IntVec2();

	std::vector<SpawnInfo> m_spawnInfos;

//public member functions
public:
	//constructor
	explicit MapDefinition(XmlElement const& element);

	//static functions
	static void InitializeMapDefs();
	static MapDefinition const* GetMapDefinition(std::string name);
};
