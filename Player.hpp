#pragma once
#include "Game/Controller.hpp"
#include "Engine/Renderer/Camera.hpp"


//forward declarations
class Game;


class Player : public Controller
{
//public member functions
public:
	//constructor and destructor
	Player(Game* owner, Map* map, int playerIndex, int xboxID);

	//game flow functions
	void Update(float deltaSeconds);

	void UpdateActor(float deltaSeconds);
	void UpdateFromControllerActor(float deltaSeconds);

	void UpdateFreeFly(float deltaSeconds);
	void UpdateFromControllerFreeFly();

	void RenderHUD() const;

	//player utilities
	Mat44 GetModelMatrix() const;

//public member variables
public:
	Camera m_playerCamera;
	Camera m_playerScreenCamera;

	float m_movementSpeed = 1.0f;
	float m_mouseTurnRate = 0.075f;
	float m_rollSpeed = 50.0f;
	float m_controllerTurnRate = 180.0f;
	bool  m_isSpeedUp;
	float m_speedUpAmount = 15.0f;

	Game* m_game = nullptr;

	Vec3 m_position;
	Vec3 m_velocity;
	EulerAngles m_orientation;
	EulerAngles m_angularVelocity;

	bool m_isFreeFly = false;

	int m_numPlayerKills = 0;
	int m_numPlayerDeaths = 0;

	int m_playerIndex = 0;

	int m_xboxControllerID = -1;
};
