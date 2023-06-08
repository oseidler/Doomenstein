#include "Game/Weapon.hpp"
#include "Game/Actor.hpp"
#include "Game/Map.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Math/MathUtils.hpp"


//
//constructor and destructor
//
Weapon::Weapon(WeaponDefinition const* definition, Actor* owner)
	: m_definition(definition)
	, m_owner(owner)
{
	m_animClock = new Clock(m_owner->m_map->m_owner->m_gameClock);
	if (m_definition->m_weaponAnimDefs.size() > 0)
	{
		m_currentAnimDef = &m_definition->m_weaponAnimDefs[0];
	}

	m_currentLightIntensity = m_definition->m_lightIntensity;
	m_currentLightRadius = m_definition->m_lightRadius;
}


Weapon::~Weapon()
{
	if (m_animClock != nullptr)
	{
		delete m_animClock;
		m_animClock = nullptr;
	}
}


//
//public weapon utilties
//
void Weapon::Fire()
{
	if (m_refireTimer > 0.0f)
	{
		return;
	}

	SetAnimationByName("Attack");

	m_holdWeaponBeingUsed = true;
	m_currentLightIntensity = m_definition->m_focusLightIntensity;
	m_currentLightRadius = m_definition->m_focusLightRadius;
	
	if (m_definition->m_focusLightRadius > 0.0f)
	{
		RaycastResultGame result = m_owner->m_map->RaycastAgainstAll(m_owner->m_position + Vec3(0.0f, 0.0f, m_owner->m_definition->m_eyeHeight), m_owner->GetTrueModelMatrix().GetIBasis3D(), m_definition->m_focusLightRange, m_owner);

		if (result.m_actorHit != nullptr && !result.m_actorHit->m_definition->m_immuneToLight)
		{
			result.m_actorHit->AddImpulse(result.m_raycastResult.m_rayDirection * m_definition->m_focusLightImpulse);

			if (m_damageIntervalTimer <= 0.0f)
			{
				m_damageIntervalTimer = m_definition->m_focusLightDamageInterval;
				result.m_actorHit->TakeDamage(m_owner->m_UID, static_cast<int>(m_definition->m_focusLightDamage));
			}
		}
	}

	if (m_definition->m_rayCount > 0)
	{
		for (int rayIndex = 0; rayIndex < m_definition->m_rayCount; rayIndex++)
		{
			Vec3 randomDirection = GetRandomDirectionInCone(m_definition->m_rayCone);

			RaycastResultGame result = m_owner->m_map->RaycastAgainstAll(m_owner->m_position + Vec3(0.0f, 0.0f, m_owner->m_definition->m_eyeHeight), randomDirection, m_definition->m_rayRange, m_owner);
			if (result.m_actorHit != nullptr)
			{
				int damageAmount = static_cast<int>(g_rng.RollRandomFloatInRange(m_definition->m_rayDamage.m_min, m_definition->m_rayDamage.m_max));

				result.m_actorHit->TakeDamage(m_owner->m_UID, damageAmount);
				result.m_actorHit->AddImpulse(result.m_raycastResult.m_rayDirection * m_definition->m_rayImpulse);

				//spawn blood splatter actor at impact position
				m_owner->m_map->SpawnActor("BloodSplatter", result.m_raycastResult.m_impactPos, EulerAngles());
			}
			else
			{
				m_owner->m_map->SpawnActor("BulletHit", result.m_raycastResult.m_impactPos, EulerAngles());
			}
		}
	}
	if (m_definition->m_projectileCount > 0)
	{
		for (int projectileIndex = 0; projectileIndex < m_definition->m_projectileCount; projectileIndex++)
		{
			Vec3 randomVelocityDirection = GetRandomDirectionInCone(m_definition->m_projectileCone);

			m_owner->m_map->SpawnProjectile(m_definition->m_projectileActor, m_owner->m_position + Vec3(0.0f, 0.0f, m_owner->m_definition->m_eyeHeight), m_owner->m_orientation, randomVelocityDirection * m_definition->m_projectileSpeed, m_owner);
		}
	}
	if (m_definition->m_meleeCount > 0)
	{
		for (int meleeIndex = 0; meleeIndex < m_definition->m_meleeCount; meleeIndex++)
		{
			float closestEnemyDistance = FLT_MAX;
			Actor* closestEnemy = nullptr;

			for (int actorIndex = 0; actorIndex < m_owner->m_map->m_allActors.size(); actorIndex++)
			{
				Actor* target = m_owner->m_map->m_allActors[actorIndex];
				if (target == nullptr)
				{
					continue;
				}

				if (m_owner->m_definition->m_faction == ActorFaction::DEMON)
				{
					if (target->m_definition->m_faction != ActorFaction::MARINE)
					{
						continue;
					}
				}
				if (m_owner->m_definition->m_faction == ActorFaction::MARINE)
				{
					if (target->m_definition->m_faction != ActorFaction::DEMON)
					{
						continue;
					}
				}

				float targetDistance = GetDistance3D(m_owner->m_position, target->m_position);
				if (targetDistance > m_definition->m_meleeRange)
				{
					continue;
				}

				Vec3 targetDisplacement = target->m_position - m_owner->m_position;
				Vec2 targetDisplacementXY = Vec2(targetDisplacement.x, targetDisplacement.y);
				float targetDisplacementAngle = GetAngleDegreesBetweenVectors2D(m_owner->GetModelMatrixYawOnly().GetIBasis2D(), targetDisplacementXY);
				if (targetDisplacementAngle > m_definition->m_meleeArc * 0.5f)
				{
					continue;
				}

				if (targetDistance < closestEnemyDistance)
				{
					closestEnemyDistance = targetDistance;
					closestEnemy = target;
				}
			}

			int damageAmount = static_cast<int>(g_rng.RollRandomFloatInRange(m_definition->m_meleeDamage.m_min, m_definition->m_meleeDamage.m_max));

			if (closestEnemy != nullptr)
			{
				closestEnemy->TakeDamage(m_owner->m_UID, damageAmount);
				closestEnemy->AddImpulse(m_owner->GetModelMatrixYawOnly().GetIBasis3D() * m_definition->m_meleeImpulse);
			}
		}
	}

	m_refireTimer = m_definition->m_refireTime;
}


//
//accessors
//
Vec3 Weapon::GetRandomDirectionInCone(float coneDegrees) const
{
	//pardon how messy of an implementation this is
	float randomPitch = g_rng.RollRandomFloatInRange(-coneDegrees * 0.5f, coneDegrees * 0.5f);
	float randomYaw = g_rng.RollRandomFloatInRange(-coneDegrees * 0.5f, coneDegrees * 0.5f);

	EulerAngles randomOrientation = EulerAngles(m_owner->m_orientation.m_yawDegrees + randomYaw, m_owner->m_orientation.m_pitchDegrees + randomPitch, m_owner->m_orientation.m_rollDegrees);
	Vec3 randomDirection = Vec3();
	Vec3 unusedJ = Vec3();
	Vec3 unusedK = Vec3();
	randomOrientation.GetAsVectors_XFwd_YLeft_ZUp(randomDirection, unusedJ, unusedK);

	return randomDirection;
}


void Weapon::SetAnimationByName(std::string animName)
{
	//no need to do anything if we're already playing that animation
	if (m_currentAnimDef != nullptr && m_currentAnimDef->m_name == animName)
	{
		if (m_definition->m_holdToUse)
		{
			m_animClock->Reset();
		}

		return;
	}

	if (animName == "Idle")
	{
		m_currentLightIntensity = m_definition->m_lightIntensity;
		m_currentLightRadius = m_definition->m_lightRadius;
	}

	for (int animIndex = 0; animIndex < m_definition->m_weaponAnimDefs.size(); animIndex++)
	{
		if (m_definition->m_weaponAnimDefs[animIndex].m_name == animName)
		{
			m_currentAnimDef = &m_definition->m_weaponAnimDefs[animIndex];
			m_animClock->Reset();
			return;
		}
	}
}
