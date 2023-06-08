#pragma once
#include "Game/ActorUID.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Audio/AudioSystem.hpp"


//forward declarations
class ActorDefinition;
class Weapon;
class Map;
class Controller;
class AI;
class SpriteAnimGroupDef;


class Actor
{
//public mumber functions
public:
	//constructor and destructor
	Actor(ActorUID uid, ActorDefinition const* definition, Map* owner, Vec3 position, EulerAngles orientation, Vec3 velocity = Vec3());
	~Actor();

	//game flow functions
	void Startup();
	void Update(float deltaSeconds);
	void Render(int currentPlayerRendering);

	//physics functions
	void UpdatePhysics(float deltaSeconds);
	void AddForce(Vec3 const& forceVector);
	void AddImpulse(Vec3 const& impulseVector);

	//callback functions
	void OnCollide();
	void OnCollideWithActor(Actor* otherActor);
	void OnPossessed(Controller* controller);
	void OnUnpossessed(Controller* controller);

	//actor utilities
	void TakeDamage(ActorUID source, int damageAmount);
	void MoveInDirection(Vec3 const& directionNormal, float speed);
	void TurnInDirection(Vec3 const& directionNormal, float maxDegrees);
	void Attack();
	void EquipWeapon(int weaponIndex);
	
	//animation functions
	void SetAnimationByName(std::string animName);

	//sound functions
	int AddSoundToSoundPlaybacks(SoundPlaybackID soundPlayback);

	//accessors
	Mat44 GetModelMatrixYawOnly() const;
	Mat44 GetTrueModelMatrix() const;

//public member variables
public:
	ActorUID m_UID;

	ActorDefinition const*  m_definition;

	std::vector<Weapon*>	m_weapons;
	Weapon*					m_currentWeapon;

	Map*   m_map;
	Actor* m_projectileOwner;

	Controller* m_currentController;
	AI*			m_AIController;
	
	Vec3 m_position;
	Vec3 m_velocity;
	Vec3 m_acceleration;
	EulerAngles m_orientation;

	bool m_isStatic = false;

	int   m_health = 100;
	bool  m_isGarbage = false;
	float m_deathTimer = 0.0f;

	Rgba8 m_color = Rgba8();

	float m_physicsHeight;
	float m_physicsRadius;

	Clock*					  m_animClock = nullptr;
	SpriteAnimGroupDef const* m_currentAnimGroup = nullptr;

	std::vector<SoundPlaybackID> m_soundPlaybacks;
	int m_weaponSoundIndex = -1;

	Mat44 m_billboardMatrix = Mat44();
};
