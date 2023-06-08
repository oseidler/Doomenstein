#pragma once
#include "Game/ActorUID.hpp"


//forward declarations
class Map;
class Actor;


class Controller
{
//public member functions
public:
	//constructor
	Controller(Map* map);

	//controller utilities
	void Possess(ActorUID uidToPossess);

	//accessors
	Actor* GetActor();

//public member variables
public:
	ActorUID m_actorUID = ActorUID(ActorUID::INVALID, ActorUID::INVALID);
	Map*	 m_map;
};
