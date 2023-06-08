#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Audio/AudioSystem.hpp"


//forward declarations
class Shader;
class Texture;
class SpriteSheet;


//anim struct
struct WeaponAnimDef
{
	WeaponAnimDef(std::string name, Shader* shader, SpriteSheet* spriteSheet, int startFrame, int endFrame, float secondsPerFrame, SpriteAnimPlaybackType playbackType)
		: m_name(name)
		, m_shader(shader)
		, m_secondsPerFrame(secondsPerFrame)
		, m_playbackMode(playbackType)
		, m_spriteSheet(spriteSheet)
	{
		m_spriteAnimDef = new SpriteAnimDefinition(*m_spriteSheet, startFrame, endFrame, 1.0f/secondsPerFrame, playbackType);
		m_numFrames = endFrame - startFrame + 1;
	}

	std::string  m_name;
	Shader*		 m_shader = nullptr;
	float		 m_secondsPerFrame = 0.0f;
	int			 m_numFrames = 0;
	SpriteAnimPlaybackType m_playbackMode = SpriteAnimPlaybackType::ONCE;
	SpriteSheet*		   m_spriteSheet = nullptr;
	SpriteAnimDefinition*  m_spriteAnimDef;
};


class WeaponDefinition
{
//public member variables
public:
	static std::vector<WeaponDefinition> s_weaponDefinitions;

	//parameters
	std::string m_name = "invalid weapon name";
	float		m_refireTime = 0.0f;
	int			m_rayCount = 0;
	float		m_rayCone = 0.0f;
	float		m_rayRange = 0.0f;
	FloatRange	m_rayDamage = FloatRange(0.0f, 0.0f);
	float		m_rayImpulse = 0.0f;
	int			m_projectileCount = 0;
	float		m_projectileCone = 0.0f;
	float		m_projectileSpeed = 0.0f;
	std::string m_projectileActor = "invalid projectile name";
	int			m_meleeCount = 0;
	float		m_meleeRange = 0.0f;
	float		m_meleeArc = 0.0f;
	FloatRange  m_meleeDamage = FloatRange(0.0f, 0.0f);
	float		m_meleeImpulse = 0.0f;
	bool		m_holdToUse = false;
	
	//flashlight parameters
	float		m_lightIntensity = 0.0f;
	float		m_focusLightIntensity = 0.0f;
	float		m_lightRadius = 0.0f;
	float		m_focusLightRadius = 0.0f;
	float		m_focusLightDamage = 0.0f;
	float		m_focusLightDamageInterval = 0.0f;
	float		m_focusLightImpulse = 0.0f;
	float		m_focusLightRange = 40.0f;

	//hud parameters
	Shader*  m_hudShader = nullptr;
	Texture* m_baseTexture = nullptr;
	Texture* m_reticleTexture = nullptr;
	Vec2     m_reticleSize = Vec2(1.0f, 1.0f);
	Vec2	 m_spriteSize = Vec2(1.0f, 1.0f);
	Vec2	 m_spritePivot = Vec2(0.5f, 0.0f);

	//anim parameters
	std::vector<WeaponAnimDef> m_weaponAnimDefs;

	//sound parameters
	std::vector<SoundID>	 m_sounds;
	std::vector<std::string> m_soundNames;

//public member functions
public:
	//constructor
	explicit WeaponDefinition(XmlElement const& element);

	//static functions
	static void InitializeWeaponDefs();
	static WeaponDefinition const* GetWeaponDefinition(std::string name);
};
