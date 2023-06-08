#include "Game/Controller.hpp"
#include "Game/Map.hpp"
#include "Game/Actor.hpp"
#include "Game/ActorDefinition.hpp"


//
//constructor
//
Controller::Controller(Map* map)
	: m_map(map)
{
}


//
//public controller utilities
//
void Controller::Possess(ActorUID uidToPossess)
{
	//unpossess previous actor
	if (m_actorUID.m_data != ActorUID::INVALID)
	{
		Actor* previousActor = m_map->GetActorByUID(m_actorUID);
		if (previousActor != nullptr)
		{
			previousActor->OnUnpossessed(this);
		}
	}

	//possess new actor
	if (uidToPossess.m_data != ActorUID::INVALID)
	{
		Actor* newActor = m_map->GetActorByUID(uidToPossess);
		if (newActor != nullptr && newActor->m_definition->m_canBePossessed)
		{
			m_actorUID = uidToPossess;
			newActor->OnPossessed(this);
		}
	}
	else
	{
		m_actorUID = ActorUID(ActorUID::INVALID, ActorUID::INVALID);
	}
}


//
//public accessors
//
Actor* Controller::GetActor()
{
	if (m_actorUID.m_data == ActorUID::INVALID)
	{
		return nullptr;
	}

	if (m_map != nullptr)
	{
		return m_map->GetActorByUID(m_actorUID);
	}

	return nullptr;
}
