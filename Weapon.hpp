#pragma once
#include "Game/WeaponDefinition.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/EngineCommon.hpp"


//forward declarations
class Actor;
struct Vec3;


class Weapon
{
//public member functions
public:
	//constructor and destructor
	explicit Weapon(WeaponDefinition const* definition, Actor* owner);
	~Weapon();
	
	//weapon utilities
	void Fire();

	//accessors
	Vec3 GetRandomDirectionInCone(float coneDegrees) const;

	//animation functions
	void SetAnimationByName(std::string animName);

//public member variables
public:
	WeaponDefinition const* m_definition;
	Actor*					m_owner;

	float m_refireTimer = 0.0f;
	float m_damageIntervalTimer = 0.0f;

	WeaponAnimDef const* m_currentAnimDef;
	Clock*				 m_animClock = nullptr;

	bool m_holdWeaponBeingUsed = false;
	float m_currentLightIntensity = 0.0f;
	float m_currentLightRadius = 0.0f;
};
