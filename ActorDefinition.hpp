#pragma once
#include "Game/SpriteAnimGroupDef.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Audio/AudioSystem.hpp"


//forward declarations
class Texture;
class SpriteSheet;
class Shader;


enum class ActorFaction
{
	NEUTRAL,
	MARINE,
	DEMON,
};


class ActorDefinition
{
//public member variables
public:
	static std::vector<ActorDefinition> s_actorDefinitions;
	static std::vector<ActorDefinition> s_projectileActorDefinitions;

	//base parameters
	std::string  m_name = "invalid actor";
	bool		 m_isVisible = false;
	int			 m_maxHealth = 1;
	float		 m_corpseLifetime = 0.0f;
	ActorFaction m_faction = ActorFaction::NEUTRAL;
	bool		 m_canBePossessed = false;
	bool		 m_dieOnSpawn = false;
	bool		 m_immuneToLight = false;

	//collision parameters
	float	   m_physicsHeight = 0.0f;
	float	   m_physicsRadius = 0.0f;
	bool	   m_collideWithWorld = false;
	bool	   m_collideWithActors = false;
	bool	   m_isPushable = true;
	bool	   m_dieOnCollide = false;
	FloatRange m_damageOnCollide = FloatRange(0.0f, 0.0f);
	float	   m_impulseOnCollide = 0.0f;

	//physics parameters
	bool  m_isSimulated = false;
	bool  m_isFlying = false;
	float m_walkSpeed = 0.0f;
	float m_runSpeed = 0.0f;
	float m_drag = 0.0f;
	float m_turnSpeed = 0.0f;

	//camera parameters
	float m_eyeHeight = 0.0f;
	float m_cameraFOVDegrees = 60.0f;
	
	//ai parameters
	bool  m_isAIEnabled = false;
	float m_sightRadius = 0.0f;
	float m_sightAngle = 0.0f;
	bool  m_freezeWhenSeen = false;

	//visuals parameters
	Vec2		  m_spriteSize = Vec2(1.0f, 1.0f);
	Vec2		  m_spritePivot = Vec2(0.5f, 0.5f);
	BillboardType m_billboardType = BillboardType::NONE;
	bool		  m_renderLit = false;
	bool		  m_renderRounded = false;
	Shader*		  m_shader = nullptr;
	Texture*	  m_spriteSheetTexture = nullptr;
	IntVec2		  m_spriteSheetCellCount = IntVec2(1, 1);
	SpriteSheet*  m_spriteSheet = nullptr;

	//animation groups definitions
	std::vector<SpriteAnimGroupDef> m_animGroupDefs;

	//sound parameters
	std::vector<SoundID>	 m_sounds;
	std::vector<std::string> m_soundNames;	//indexes should always align for sounds and sound names

	//weapon parameters
	std::vector<std::string> m_weapons;

//public member functions
public:
	//constructor
	explicit ActorDefinition(XmlElement const& element);

	//static functions
	static void InitializeActorDefs();
	static void InitializeProjectileActorDefs();
	static ActorDefinition const* GetActorDefinition(std::string name);
	static ActorDefinition const* GetProjectileActorDefinition(std::string name);
};
