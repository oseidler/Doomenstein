#pragma once
#include "Game/Controller.hpp"


class AI : public Controller
{
//public member functions
public:
	//constructor
	AI(Map* map);

	//game flow functions
	void Update(float deltaSeconds);

	//AI functions
	void DamagedBy(ActorUID damager);

//public member variables
public:
	ActorUID m_targetUID = ActorUID(ActorUID::INVALID, ActorUID::INVALID);
};
