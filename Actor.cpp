#include "Game/Actor.hpp"
#include "Game/Game.hpp"
#include "Game/ActorDefinition.hpp"
#include "Game/WeaponDefinition.hpp"
#include "Game/Weapon.hpp"
#include "Game/Map.hpp"
#include "Game/Controller.hpp"
#include "Game/AI.hpp"
#include "Game/Player.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"


//
//constructor and destructor
//
Actor::Actor(ActorUID uid, ActorDefinition const* definition, Map* map, Vec3 position, EulerAngles orientation, Vec3 velocity)
	: m_UID(uid)
	, m_definition(definition)
	, m_map(map)
	, m_position(position)
	, m_orientation(orientation)
	, m_physicsHeight(definition->m_physicsHeight)
	, m_physicsRadius(definition->m_physicsRadius)
	, m_velocity(velocity)
	, m_health(definition->m_maxHealth)
{
	for (int weaponIndex = 0; weaponIndex < m_definition->m_weapons.size(); weaponIndex++)
	{
		WeaponDefinition const* weaponDefinition = WeaponDefinition::GetWeaponDefinition(m_definition->m_weapons[weaponIndex]);
		
		m_weapons.push_back(new Weapon(weaponDefinition, this));
	}
	if (m_weapons.size() > 0)
	{
		m_currentWeapon = m_weapons[0];
	}

	if (!m_definition->m_isPushable)
	{
		m_isStatic = true;
	}
}


Actor::~Actor()
{
	if (m_animClock != nullptr)
	{
		delete m_animClock;
		m_animClock = nullptr;
	}
	
	for (int weaponIndex = 0; weaponIndex < m_weapons.size(); weaponIndex++)
	{
		Weapon* weapon = m_weapons[weaponIndex];

		if (weapon != nullptr)
		{
			delete weapon;
			weapon = nullptr;
		}
	}

	if (m_AIController != nullptr)
	{
		delete m_AIController;
		m_AIController = nullptr;
	}
}


//
//public game flow functions
//
void Actor::Startup()
{
	if (m_definition->m_isAIEnabled)
	{
		AI* aiController = new AI(m_map);
		m_AIController = aiController;
		aiController->Possess(m_UID);
	}

	if (m_definition->m_dieOnSpawn)
	{
		m_health = 0;
	}

	m_animClock = new Clock(m_map->m_owner->m_gameClock);

	if (m_definition->m_animGroupDefs.size() > 0)
	{
		m_currentAnimGroup = &m_definition->m_animGroupDefs[0];
	}
}


void Actor::Update(float deltaSeconds)
{
	if (m_AIController != nullptr && m_currentController == m_AIController)
	{
		m_AIController->Update(deltaSeconds);
	}
	
	if (m_health <= 0 && m_definition->m_corpseLifetime > 0.0f)
	{
		if (m_deathTimer <= 0.0f)
		{
			if (m_definition->m_sounds.size() > 1)
			{
				SoundPlaybackID soundPlayback = g_theAudio->StartSoundAt(m_definition->m_sounds[1], m_position);
				AddSoundToSoundPlaybacks(soundPlayback);
			}
		}
		m_deathTimer += deltaSeconds;
		SetAnimationByName("Death");
	}
	if (m_deathTimer >= m_definition->m_corpseLifetime && m_definition->m_corpseLifetime > 0.0f)
	{
		m_isGarbage = true;
	}
	
	if (m_definition->m_isSimulated && m_health > 0)
	{
		UpdatePhysics(deltaSeconds);
	}

	/*if (!m_definition->m_isFlying)
	{
		m_position.z = 0.0f;
	}*/

	for (int weaponIndex = 0; weaponIndex < m_weapons.size(); weaponIndex++)
	{
		Weapon* weapon = m_weapons[weaponIndex];
		if (weapon != nullptr)
		{
			if (weapon->m_refireTimer > 0.0f)
			{
				weapon->m_refireTimer -= deltaSeconds;
			}
			if (weapon->m_damageIntervalTimer > 0.0f)
			{
				weapon->m_damageIntervalTimer -= deltaSeconds;
			}
		}
	}

	if (m_currentWeapon != nullptr && m_currentWeapon->m_definition->m_holdToUse)
	{
		if (m_currentWeapon->m_holdWeaponBeingUsed)
		{
			m_currentWeapon->m_holdWeaponBeingUsed = false;
		}
		else
		{
			if (m_weaponSoundIndex < m_soundPlaybacks.size())
			{
				g_theAudio->StopSound(m_soundPlaybacks[m_weaponSoundIndex]);
				m_weaponSoundIndex = -1;
			}
			m_currentWeapon->SetAnimationByName("Idle");
		}
	}

	//sound updating
	for (int soundIndex = 0; soundIndex < m_soundPlaybacks.size(); soundIndex++)
	{
		if (g_theAudio->IsPlaying(m_soundPlaybacks[soundIndex]))
		{
			g_theAudio->SetSoundPosition(m_soundPlaybacks[soundIndex], m_position);
		}
	}
}


void Actor::Render(int currentPlayerRendering)
{
	if (m_definition->m_isVisible)
	{
		std::vector<Vertex_PNCU> actorVerts;

		//don't render the actor currently controlled by the player
		if (m_currentController != nullptr && m_map->m_currentPlayerActors[currentPlayerRendering] == this)
		{
			return;
		}

		//don't render if we don't have a current anim group
		if (m_currentAnimGroup == nullptr)
		{
			return;
		}

		if (m_animClock->GetTotalSeconds() > m_currentAnimGroup->m_secondsPerFrame * m_currentAnimGroup->m_numFrames && m_currentAnimGroup->m_playbackMode == SpriteAnimPlaybackType::ONCE)
		{
			SetAnimationByName("Walk");
		}

		if (m_currentAnimGroup->m_scaleBySpeed)
		{
			m_animClock->SetTimeScale(m_velocity.GetLength() / m_definition->m_runSpeed);
		}
		else
		{
			m_animClock->SetTimeScale(1.0f);
		}

		int direction = 0;
		float maxDotProduct = -FLT_MAX;
		for (int directionIndex = 0; directionIndex < m_currentAnimGroup->m_directions.size(); directionIndex++)
		{
			Vec3 cameraViewVector = m_position - m_map->m_players[currentPlayerRendering]->m_playerCamera.GetCameraPosition();
			cameraViewVector.z = 0.0f;
			cameraViewVector.Normalize();
			cameraViewVector = GetTrueModelMatrix().GetOrthonormalInverse().TransformVectorQuantity3D(cameraViewVector);

			float dotProduct = DotProduct3D(m_currentAnimGroup->m_directions[directionIndex], cameraViewVector);

			if (dotProduct > maxDotProduct)
			{
				maxDotProduct = dotProduct;
				direction = directionIndex;
			}
		}

		SpriteDefinition const& spriteDef = m_currentAnimGroup->m_spriteAnimDefs[direction].GetSpriteDefAtTime(m_animClock->GetTotalSeconds());
		AABB2 spriteUVs = spriteDef.GetUVs();

		Vec3 spriteBottomLeft = Vec3();
		Vec3 spriteBottomRight = spriteBottomLeft + Vec3(0.0f, m_definition->m_spriteSize.x, 0.0f);
		Vec3 spriteTopLeft = spriteBottomLeft + Vec3(0.0f, 0.0f, m_definition->m_spriteSize.y);
		Vec3 spriteTopRight = spriteBottomRight + Vec3(0.0f, 0.0f, m_definition->m_spriteSize.y);
		if (m_definition->m_renderRounded)
		{
			AddVertsForRoundedQuad3D(actorVerts, spriteBottomLeft, spriteBottomRight, spriteTopLeft, spriteTopRight, Rgba8(), spriteUVs);
		}
		else
		{
			AddVertsForQuad3D(actorVerts, spriteBottomLeft, spriteBottomRight, spriteTopLeft, spriteTopRight, Rgba8(), spriteUVs);
		}

		Vec3 pivotTranslation = (Vec3() - Vec3(0.0f, m_definition->m_spritePivot.x * m_definition->m_spriteSize.x, m_definition->m_spritePivot.y * m_definition->m_spriteSize.y));
		Mat44 pivotTranslationMatrix = Mat44::CreateTranslation3D(pivotTranslation);
		TransformVertexArray3D(actorVerts, pivotTranslationMatrix);

		Mat44 cameraMatrix = m_map->m_players[currentPlayerRendering]->m_playerCamera.GetViewMatrix().GetOrthonormalInverse();
		m_billboardMatrix = GetBillboardMatrix(m_definition->m_billboardType, cameraMatrix, m_position);
		TransformVertexArray3D(actorVerts, m_billboardMatrix);

		Mat44 translationMatrix = Mat44::CreateTranslation3D(m_position);
		TransformVertexArray3D(actorVerts, translationMatrix);

		g_theRenderer->BindShader(m_definition->m_shader);
		g_theRenderer->BindTexture(&m_definition->m_spriteSheet->GetTexture());
		g_theRenderer->SetModelConstants();
		g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
		g_theRenderer->DrawVertexArray(static_cast<int>(actorVerts.size()), actorVerts.data());
	}
}


//
//public physics functions
//
void Actor::UpdatePhysics(float deltaSeconds)
{
	if (!m_definition->m_isFlying)
	{
		m_velocity.z = 0.0f;
	}

	Vec3 dragForce = (Vec3() - m_velocity) * m_definition->m_drag;
	AddForce(dragForce);
	
	m_velocity += m_acceleration * deltaSeconds;
	m_position += m_velocity * deltaSeconds;

	m_acceleration = Vec3();
}


void Actor::AddForce(Vec3 const& forceVector)
{
	m_acceleration += forceVector;
}


void Actor::AddImpulse(Vec3 const& impulseVector)
{
	m_velocity += impulseVector;
}


//
//public callback functions
//
void Actor::OnCollide()
{
	if (m_definition->m_dieOnCollide)
	{
		m_health = 0;
	}
}


void Actor::OnCollideWithActor(Actor* otherActor)
{
	int damageAmount = static_cast<int>(g_rng.RollRandomFloatInRange(m_definition->m_damageOnCollide.m_min, m_definition->m_damageOnCollide.m_max));

	if (otherActor != nullptr)
	{
		if (m_projectileOwner != nullptr)
		{
			otherActor->TakeDamage(m_projectileOwner->m_UID, damageAmount);
		}
		else
		{
			otherActor->TakeDamage(m_UID, damageAmount);
		}

		otherActor->AddImpulse(GetModelMatrixYawOnly().GetIBasis3D() * m_definition->m_impulseOnCollide);
	}
	
	OnCollide();
}


void Actor::OnPossessed(Controller* controller)
{
	m_currentController = controller;
}


void Actor::OnUnpossessed(Controller* controller)
{
	//----------------
	//INCOMPLETE
	//----------------

	m_currentController = nullptr;

	UNUSED(controller);
}


void Actor::TakeDamage(ActorUID source, int damageAmount)
{
	if (m_health > 0 && damageAmount > 0)
	{
		m_health -= damageAmount;

		if (m_AIController != nullptr && m_AIController == m_currentController && damageAmount > 0)
		{
			m_AIController->DamagedBy(source);
		}

		SetAnimationByName("Hurt");

		if (m_health <= 0)
		{
			m_health = 0;

			Actor* damageSource = m_map->GetActorByUID(source);

			if (this == m_map->m_currentPlayerActors[0] && damageSource != nullptr && damageSource == m_map->m_currentPlayerActors[1])
			{
				m_map->m_players[0]->m_numPlayerDeaths += 1;
				m_map->m_players[1]->m_numPlayerKills += 1;
			}
			else if (this == m_map->m_currentPlayerActors[1] && damageSource != nullptr && damageSource == m_map->m_currentPlayerActors[0])
			{
				m_map->m_players[1]->m_numPlayerDeaths += 1;
				m_map->m_players[0]->m_numPlayerKills += 1;
			}
		}

		//damage sound
		if (m_definition->m_sounds.size() > 0 && m_health > 0)
		{
			SoundPlaybackID soundPlayback = g_theAudio->StartSoundAt(m_definition->m_sounds[0], m_position);
			AddSoundToSoundPlaybacks(soundPlayback);
		}
	}
}


//
//public actor utilities
//
void Actor::MoveInDirection(Vec3 const& directionNormal, float speed)
{
	Vec3 forceVector = directionNormal.GetNormalized() * speed * m_definition->m_drag;
	
	AddForce(forceVector);
}


void Actor::TurnInDirection(Vec3 const& directionNormal, float maxDegrees)
{
	Vec2 forwardNormalXY = GetModelMatrixYawOnly().GetIBasis2D();
	float currentDegrees = forwardNormalXY.GetOrientationDegrees();

	Vec2 directionNormalXY = Vec2(directionNormal.x, directionNormal.y);
	float goalDegrees = directionNormalXY.GetOrientationDegrees();

	m_orientation.m_yawDegrees = GetTurnedTowardDegrees(currentDegrees, goalDegrees, maxDegrees);
}


void Actor::Attack()
{
	if (m_currentWeapon != nullptr && m_currentWeapon->m_refireTimer <= 0.0f)
	{
		m_currentWeapon->Fire();

		//attack sound
		if (m_currentWeapon->m_definition->m_sounds.size() > 0)
		{
			if (m_currentWeapon->m_definition->m_holdToUse)
			{
				if (m_weaponSoundIndex == -1)
				{
					SoundPlaybackID soundPlayback = g_theAudio->StartSoundAt(m_currentWeapon->m_definition->m_sounds[0], m_position, true, 0.2f);
					m_weaponSoundIndex = AddSoundToSoundPlaybacks(soundPlayback);
				}
			}
			else
			{
				SoundPlaybackID soundPlayback = g_theAudio->StartSoundAt(m_currentWeapon->m_definition->m_sounds[0], m_position);
				AddSoundToSoundPlaybacks(soundPlayback);
			}
		}
	}
}


void Actor::EquipWeapon(int weaponIndex)
{
	if (weaponIndex < m_weapons.size())
	{
		m_currentWeapon = m_weapons[weaponIndex];
	}
}


//
//animation functions
//
void Actor::SetAnimationByName(std::string animName)
{
	//no need to do anything if we're already playing that animation
	if (m_currentAnimGroup->m_name == animName)
	{
		return;
	}

	for (int groupIndex = 0; groupIndex < m_definition->m_animGroupDefs.size(); groupIndex++)
	{
		if (m_definition->m_animGroupDefs[groupIndex].m_name == animName)
		{
			m_currentAnimGroup = &m_definition->m_animGroupDefs[groupIndex];
			m_animClock->Reset();
			return;
		}
	}
}


//
//sound functions
//
int Actor::AddSoundToSoundPlaybacks(SoundPlaybackID soundPlayback)
{
	for (int playbackIndex = 0; playbackIndex < m_soundPlaybacks.size(); playbackIndex++)
	{
		if (!g_theAudio->IsPlaying(m_soundPlaybacks[playbackIndex]))
		{
			m_soundPlaybacks[playbackIndex] = soundPlayback;
			return playbackIndex;
		}
	}

	m_soundPlaybacks.push_back(soundPlayback);
	return static_cast<int>(m_soundPlaybacks.size()) - 1;
}


//
//public accessors
//
Mat44 Actor::GetModelMatrixYawOnly() const
{
	EulerAngles yawOnlyOrient = EulerAngles(m_orientation.m_yawDegrees, 0.0f, 0.0f);
	
	Mat44 modelMatrix = yawOnlyOrient.GetAsMatrix_XFwd_YLeft_ZUp();

	modelMatrix.SetTranslation3D(m_position);

	return modelMatrix;
}


Mat44 Actor::GetTrueModelMatrix() const
{
	Mat44 modelMatrix = m_orientation.GetAsMatrix_XFwd_YLeft_ZUp();

	modelMatrix.SetTranslation3D(m_position);

	return modelMatrix;
}
