#pragma once
#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/Vec3.hpp"


//forward declarations
class ActorDefinition;


class SpriteAnimGroupDef
{
//public member functions
public:
	//constructor
	SpriteAnimGroupDef(ActorDefinition* actorDefinition);

	//parsing function
	void LoadFromXMLElement(XmlElement const& element);

//public member variables
public:
	std::vector<SpriteAnimDefinition> m_spriteAnimDefs;	
	std::vector<Vec3>				  m_directions;			//all directions are at same index as associated sprite anim def
	ActorDefinition*				  m_actorDefinition;

	//base properties
	std::string			   m_name;
	bool				   m_scaleBySpeed = false;
	float				   m_secondsPerFrame = 1.0f;
	SpriteAnimPlaybackType m_playbackMode = SpriteAnimPlaybackType::ONCE;
	
	int m_numFrames = 1;
};
