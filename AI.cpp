#include "Game/AI.hpp"
#include "Game/Map.hpp"
#include "Game/Actor.hpp"
#include "Game/ActorDefinition.hpp"
#include "Game/WeaponDefinition.hpp"
#include "Game/Weapon.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"


//
//constructor
//
AI::AI(Map* map)
	: Controller(map)
{
}


//
//game flow functions
//
void AI::Update(float deltaSeconds)
{
	Actor* thisActor = m_map->GetActorByUID(m_actorUID);

	if (thisActor->m_health <= 0)
	{
		return;
	}

	Actor* targetActor = nullptr;
	
	if (m_targetUID.m_data == ActorUID::INVALID)
	{
		if (thisActor->m_definition->m_faction == ActorFaction::DEMON)
		{
			targetActor = m_map->GetClosestVisibleEnemy(ActorFaction::MARINE, thisActor);
		}
		else if (thisActor->m_definition->m_faction == ActorFaction::MARINE)
		{
			targetActor = m_map->GetClosestVisibleEnemy(ActorFaction::DEMON, thisActor);
		}
		if (targetActor != nullptr)
		{
			m_targetUID = targetActor->m_UID;
		}
	}
	else
	{ 
		targetActor = m_map->GetActorByUID(m_targetUID);
	}
	
	
	if (targetActor != nullptr && thisActor != nullptr)
	{
		//don't go through with moving, turning, or attacking logic if being looked at
		if (thisActor->m_definition->m_freezeWhenSeen)
		{
			Vec3 targetFacingDirection = targetActor->GetModelMatrixYawOnly().GetIBasis3D();
			Vec3 eyeHeightVector = Vec3(0.0f, 0.0f, 1.0f) * targetActor->m_definition->m_eyeHeight;
			Vec3 targetToSelf = thisActor->m_position - targetActor->m_position;

			//Vec3 thisActorJBasisLeft = thisActor->GetModelMatrixYawOnly().GetJBasis3D() * thisActor->m_physicsRadius;
			Vec3 thisActorJBasisLeft = thisActor->m_billboardMatrix.GetJBasis3D() * thisActor->m_physicsRadius;
			Vec3 startPointLeft = thisActor->m_position + thisActorJBasisLeft + eyeHeightVector;
			Vec3 startPointLeftToTarget = (targetActor->m_position + eyeHeightVector - startPointLeft).GetNormalized();
			Vec3 startPointRight = thisActor->m_position - thisActorJBasisLeft + eyeHeightVector;
			Vec3 startPointRightToTarget = (targetActor->m_position + eyeHeightVector - startPointRight).GetNormalized();

			RaycastResult3D raycastWallLeft = m_map->RaycastAgainstTilesXY(startPointLeft, startPointLeftToTarget, thisActor->m_definition->m_sightRadius);
			RaycastResultGame raycastTargetLeft = m_map->RaycastAgainstPlayers(startPointLeft, startPointLeftToTarget, thisActor->m_definition->m_sightRadius, thisActor);
			RaycastResult3D raycastWallRight = m_map->RaycastAgainstTilesXY(startPointRight, startPointRightToTarget, thisActor->m_definition->m_sightRadius);
			RaycastResultGame raycastTargetRight = m_map->RaycastAgainstPlayers(startPointRight, startPointRightToTarget, thisActor->m_definition->m_sightRadius, thisActor);

			if (DotProduct3D(targetFacingDirection, targetToSelf) > 0.0f && 
				((raycastWallLeft.m_impactDist > raycastTargetLeft.m_raycastResult.m_impactDist && raycastTargetLeft.m_actorHit == targetActor) 
					|| (raycastWallRight.m_impactDist > raycastTargetRight.m_raycastResult.m_impactDist && raycastTargetRight.m_actorHit == targetActor)))
			{
				thisActor->m_velocity = Vec3(0.0f, 0.0f, 0.0f);
				thisActor->m_acceleration = Vec3(0.0f, 0.0f, 0.0f);
				return;
			}
		}

		float distToTarget = GetDistance3D(thisActor->m_position, targetActor->m_position);
		if (distToTarget > thisActor->m_physicsRadius + targetActor->m_physicsRadius + 0.01f)
		{
			thisActor->MoveInDirection(thisActor->GetModelMatrixYawOnly().GetIBasis3D(), thisActor->m_definition->m_runSpeed);
		}
		thisActor->TurnInDirection((targetActor->m_position - thisActor->m_position).GetNormalized(), thisActor->m_definition->m_turnSpeed * deltaSeconds);

		if (thisActor->m_currentWeapon != nullptr && thisActor->m_currentWeapon->m_definition->m_meleeCount > 0 && distToTarget < thisActor->m_currentWeapon->m_definition->m_meleeRange)
		{
			thisActor->Attack();
		}
	}
}


//
//AI functions
//
void AI::DamagedBy(ActorUID damager)
{
	m_targetUID = damager;
}
