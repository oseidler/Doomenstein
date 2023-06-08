#include "Game/Player.hpp"
#include "Game/Game.hpp"
#include "Game/Map.hpp"
#include "Game/Actor.hpp"
#include "Game/ActorDefinition.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Weapon.hpp"
#include "Game/WeaponDefinition.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/BitmapFont.hpp"


//
//constructor
//
Player::Player(Game* owner, Map* map, int playerIndex, int xboxID)
	: m_game(owner)
	, m_playerIndex(playerIndex)
	, m_xboxControllerID(xboxID)
	, Controller(map)
{
}


//
//public game flow functions
//
void Player::Update(float deltaSeconds)
{
	if (g_theInput->WasKeyJustPressed('F') && m_map->m_players[1] == nullptr)	//can't free fly when player 2 exists
	{
		m_isFreeFly = !m_isFreeFly;

		if (!m_isFreeFly)
		{
			Possess(m_map->m_currentPlayerActors[0]->m_UID);
		}
		else
		{
			Possess(ActorUID(ActorUID::INVALID, ActorUID::INVALID));
		}
	}

	if (g_theInput->WasKeyJustPressed('N') && m_map->m_players[1] == nullptr)	//can't debug possess when player 2 exists
	{
		m_map->DebugPossessNext();
	}

	if (m_isFreeFly)
	{
		float systemDeltaSeconds = Clock::GetSystemClock().GetDeltaSeconds();
		UpdateFreeFly(systemDeltaSeconds);
	}
	else
	{
		UpdateActor(deltaSeconds);
	}
}


void Player::UpdateActor(float deltaSeconds)
{
	Actor* playerActor = m_map->GetActorByUID(m_actorUID);
	if (playerActor == nullptr)
	{
		return;
	}

	if (playerActor->m_health <= 0)
	{
		if (m_position.z > 0.0f)
		{
			float fractionFallen = playerActor->m_deathTimer / (playerActor->m_definition->m_corpseLifetime * 0.5f);

			m_position.z = Interpolate(playerActor->m_definition->m_eyeHeight, 0.0f, fractionFallen);
		}
		
		m_playerCamera.SetTransform(m_position, m_orientation);

		return;
	}

	m_isSpeedUp = false;

	if (m_xboxControllerID == -1)
	{
		Vec3 movementIntentions = Vec3();

		if (g_theInput->IsKeyDown('W'))
		{
			movementIntentions.x += 1.0f;
		}
		if (g_theInput->IsKeyDown('S'))
		{
			movementIntentions.x -= 1.0f;
		}
		if (g_theInput->IsKeyDown('A'))
		{
			movementIntentions.y += 1.0f;
		}
		if (g_theInput->IsKeyDown('D'))
		{
			movementIntentions.y -= 1.0f;
		}
		if (g_theInput->IsKeyDown('Z'))
		{
			movementIntentions.z += 1.0f;
		}
		if (g_theInput->IsKeyDown('C'))
		{
			movementIntentions.z -= 1.0f;
		}

		if (g_theInput->IsKeyDown(KEYCODE_SHIFT))
		{
			m_isSpeedUp = true;
		}

		if (g_theInput->IsKeyDown(KEYCODE_LMB))
		{
			m_map->GetActorByUID(m_actorUID)->Attack();
		}

		if (g_theInput->WasKeyJustPressed('1'))
		{
			playerActor->EquipWeapon(0);
		}
		if (g_theInput->WasKeyJustPressed('2'))
		{
			playerActor->EquipWeapon(1);
		}

		if (g_theInput->WasKeyJustPressed(KEYCODE_LEFT))
		{
			for (int weaponIndex = 0; weaponIndex < playerActor->m_weapons.size(); weaponIndex++)
			{
				if (playerActor->m_currentWeapon == playerActor->m_weapons[weaponIndex])
				{
					if (weaponIndex == 0)
					{
						playerActor->EquipWeapon(static_cast<int>(playerActor->m_weapons.size() - 1));
					}
					else
					{
						playerActor->EquipWeapon(weaponIndex - 1);
					}
					break;
				}
			}
		}
		if (g_theInput->WasKeyJustPressed(KEYCODE_RIGHT))
		{
			for (int weaponIndex = 0; weaponIndex < playerActor->m_weapons.size(); weaponIndex++)
			{
				if (playerActor->m_currentWeapon == playerActor->m_weapons[weaponIndex])
				{
					if (weaponIndex == static_cast<int>(playerActor->m_weapons.size() - 1))
					{
						playerActor->EquipWeapon(0);
					}
					else
					{
						playerActor->EquipWeapon(weaponIndex + 1);
					}
					break;
				}
			}
		}

		if (movementIntentions != Vec3())
		{
			movementIntentions.Normalize();
			movementIntentions = playerActor->GetModelMatrixYawOnly().TransformVectorQuantity3D(movementIntentions);

			if (m_isSpeedUp)
			{
				playerActor->MoveInDirection(movementIntentions, playerActor->m_definition->m_runSpeed);
			}
			else
			{
				playerActor->MoveInDirection(movementIntentions, playerActor->m_definition->m_walkSpeed);
			}
		}

		IntVec2 mouseDelta = g_theInput->GetCursorClientDelta();

		playerActor->m_orientation.m_yawDegrees -= (float)mouseDelta.x * m_mouseTurnRate;
		playerActor->m_orientation.m_pitchDegrees += (float)mouseDelta.y * m_mouseTurnRate;

		playerActor->m_orientation.m_pitchDegrees = GetClamped(playerActor->m_orientation.m_pitchDegrees, -85.0f, 85.0f);
	}
	else
	{
		UpdateFromControllerActor(deltaSeconds);
	}

	m_position = playerActor->m_position + Vec3(0.0f, 0.0f, playerActor->m_definition->m_eyeHeight);
	m_orientation = playerActor->m_orientation;

	m_playerCamera.SetTransform(m_position, m_orientation);
	m_playerCamera.SetFieldOfViewDegrees(playerActor->m_definition->m_cameraFOVDegrees);

	UNUSED(deltaSeconds);
}


void Player::UpdateFromControllerActor(float deltaSeconds)
{
	XboxController const& controller = g_theInput->GetController(0);
	AnalogJoystick const& leftStick = controller.GetLeftStick();
	AnalogJoystick const& rightStick = controller.GetRightStick();

	float leftStickMagnitude = leftStick.GetMagnitude();
	float rightStickMagnitude = rightStick.GetMagnitude();

	Actor* playerActor = m_map->GetActorByUID(m_actorUID);
	if (playerActor == nullptr)
	{
		return;
	}

	Vec3 movementIntentions = Vec3();

	if (controller.IsButtonDown(XBOX_BUTTON_A))
	{
		m_isSpeedUp = true;
	}

	if (leftStickMagnitude > 0.0f)
	{
		movementIntentions.y = -leftStick.GetPosition().x;
		movementIntentions.x = leftStick.GetPosition().y;

		movementIntentions.Normalize();
		movementIntentions = playerActor->GetModelMatrixYawOnly().TransformVectorQuantity3D(movementIntentions);

		if (m_isSpeedUp)
		{
			playerActor->MoveInDirection(movementIntentions, playerActor->m_definition->m_runSpeed);
		}
		else
		{
			playerActor->MoveInDirection(movementIntentions, playerActor->m_definition->m_walkSpeed);
		}
	}

	if (rightStickMagnitude > 0.0f)
	{
		playerActor->m_orientation.m_yawDegrees += -rightStick.GetPosition().x * playerActor->m_definition->m_turnSpeed * deltaSeconds;
		playerActor->m_orientation.m_pitchDegrees += -rightStick.GetPosition().y * playerActor->m_definition->m_turnSpeed * deltaSeconds;
	}

	if (controller.GetRightTrigger() > 0.0f)
	{
		m_map->GetActorByUID(m_actorUID)->Attack();
	}

	if (controller.WasButtonJustPressed(XBOX_BUTTON_X))
	{
		playerActor->EquipWeapon(0);
	}
	if (controller.WasButtonJustPressed(XBOX_BUTTON_Y))
	{
		playerActor->EquipWeapon(1);
	}

	if (controller.WasButtonJustPressed(XBOX_BUTTON_DOWN))
	{
		if (playerActor->m_currentWeapon == playerActor->m_weapons[0])
		{
			playerActor->EquipWeapon(1);
		}
		else if (playerActor->m_weapons.size() > 1 && playerActor->m_currentWeapon == playerActor->m_weapons[1])
		{
			playerActor->EquipWeapon(0);
		}
	}
}


void Player::UpdateFreeFly(float deltaSeconds)
{
	m_isSpeedUp = false;

	Vec3 movementIntentions = Vec3();
	int rollIntentions = 0;
	m_angularVelocity = EulerAngles(0.0f, 0.0f, 0.0f);

	if (g_theInput->IsKeyDown('W'))
	{
		movementIntentions.x += 1.0f;
	}
	if (g_theInput->IsKeyDown('S'))
	{
		movementIntentions.x -= 1.0f;
	}
	if (g_theInput->IsKeyDown('A'))
	{
		movementIntentions.y += 1.0f;
	}
	if (g_theInput->IsKeyDown('D'))
	{
		movementIntentions.y -= 1.0f;
	}
	if (g_theInput->IsKeyDown('Z'))
	{
		movementIntentions.z += 1.0f;
	}
	if (g_theInput->IsKeyDown('C'))
	{
		movementIntentions.z -= 1.0f;
	}

	if (g_theInput->WasKeyJustPressed('H'))
	{
		m_position = Vec3();
		m_orientation = EulerAngles();
	}

	if (g_theInput->IsKeyDown('Q'))
	{
		rollIntentions -= 1;
	}
	if (g_theInput->IsKeyDown('E'))
	{
		rollIntentions += 1;
	}

	if (g_theInput->IsKeyDown(KEYCODE_SHIFT))
	{
		m_isSpeedUp = true;
	}

	/*if (g_theInput->WasKeyJustPressed(KEYCODE_LMB))
	{
		Map* currentMap = m_game->m_currentMap;
		currentMap->RaycastAgainstAll(m_position, GetModelMatrix().GetIBasis3D(), 10.0f);
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_RMB))
	{
		Map* currentMap = m_game->m_currentMap;
		currentMap->RaycastAgainstAll(m_position, GetModelMatrix().GetIBasis3D(), 0.25f);
	}*/

	Vec3 iBasis = GetModelMatrix().GetIBasis3D();
	Vec3 jBasis = GetModelMatrix().GetJBasis3D();

	m_velocity = (iBasis * movementIntentions.x + jBasis * movementIntentions.y + Vec3(0.0f, 0.0f, movementIntentions.z)) * m_movementSpeed;
	m_angularVelocity.m_rollDegrees = rollIntentions * m_rollSpeed;

	IntVec2 mouseDelta = g_theInput->GetCursorClientDelta();

	m_orientation.m_yawDegrees -= (float)mouseDelta.x * m_mouseTurnRate;
	m_orientation.m_pitchDegrees += (float)mouseDelta.y * m_mouseTurnRate;

	UpdateFromControllerFreeFly();

	if (m_isSpeedUp)
	{
		m_velocity *= m_speedUpAmount;
	}

	m_orientation += m_angularVelocity * deltaSeconds;
	m_position += m_velocity * deltaSeconds;

	m_orientation.m_pitchDegrees = GetClamped(m_orientation.m_pitchDegrees, -85.0f, 85.0f);
	m_orientation.m_rollDegrees = GetClamped(m_orientation.m_rollDegrees, -45.0f, 45.0f);

	m_playerCamera.SetTransform(m_position, m_orientation);
}


void Player::UpdateFromControllerFreeFly()
{
	XboxController const& controller = g_theInput->GetController(0);
	AnalogJoystick const& leftStick = controller.GetLeftStick();
	AnalogJoystick const& rightStick = controller.GetRightStick();

	float leftStickMagnitude = leftStick.GetMagnitude();
	float rightStickMagnitude = rightStick.GetMagnitude();

	Vec3 movementIntentions = Vec3();

	if (leftStickMagnitude > 0.0f)
	{
		movementIntentions.y = -leftStick.GetPosition().x;
		movementIntentions.x = leftStick.GetPosition().y;

		Vec3 iBasis = GetModelMatrix().GetIBasis3D();
		Vec3 jBasis = GetModelMatrix().GetJBasis3D();
		m_velocity = (iBasis * movementIntentions.x + jBasis * movementIntentions.y) * m_movementSpeed;
	}

	if (controller.IsButtonDown(XBOX_BUTTON_L))
	{
		movementIntentions.z += 1.0f;
		m_velocity += Vec3(0.0f, 0.0f, movementIntentions.z) * m_movementSpeed;
	}
	if (controller.IsButtonDown(XBOX_BUTTON_R))
	{
		movementIntentions.z -= 1.0f;
		m_velocity += Vec3(0.0f, 0.0f, movementIntentions.z) * m_movementSpeed;
	}

	if (rightStickMagnitude > 0.0f)
	{
		m_angularVelocity.m_yawDegrees = -rightStick.GetPosition().x * m_controllerTurnRate;
		m_angularVelocity.m_pitchDegrees = -rightStick.GetPosition().y * m_controllerTurnRate;
	}

	if (controller.GetLeftTrigger() > 0.0f)
	{
		m_angularVelocity.m_rollDegrees = - m_rollSpeed;
	}
	if (controller.GetRightTrigger() > 0.0f)
	{
		m_angularVelocity.m_rollDegrees = m_rollSpeed;
	}

	if (controller.WasButtonJustPressed(XBOX_BUTTON_START))
	{
		m_position = Vec3();
		m_orientation = EulerAngles();
	}

	if (controller.IsButtonDown(XBOX_BUTTON_A))
	{
		m_isSpeedUp = true;
	}
}


void Player::RenderHUD() const
{
	if (m_map->m_currentPlayerActors[m_playerIndex] == nullptr)
	{
		return;
	}

	Actor* playerActor = m_map->GetActorByUID(m_actorUID);
	if (playerActor == nullptr)
	{
		return;
	}

	g_theRenderer->BeginCamera(m_playerScreenCamera);

	WeaponDefinition const* weaponDef = m_map->m_currentPlayerActors[m_playerIndex]->m_currentWeapon->m_definition;

	//render weapon
	if (m_map->m_currentPlayerActors[m_playerIndex]->m_health > 0)
	{
		int weaponAnimIndex = 0;

		if (playerActor == nullptr)
		{
			return;
		}
		Weapon* playerWeapon = playerActor->m_currentWeapon;

		if (playerWeapon->m_currentAnimDef == nullptr)
		{
			return;
		}

		if (playerWeapon->m_animClock->GetTotalSeconds() > playerWeapon->m_currentAnimDef->m_secondsPerFrame * playerWeapon->m_currentAnimDef->m_numFrames && playerWeapon->m_currentAnimDef->m_playbackMode == SpriteAnimPlaybackType::ONCE)
		{
			playerWeapon->SetAnimationByName("Idle");
		}

		SpriteDefinition const& spriteDef = playerWeapon->m_currentAnimDef->m_spriteAnimDef->GetSpriteDefAtTime(playerWeapon->m_animClock->GetTotalSeconds());
		AABB2 spriteUVs = spriteDef.GetUVs();

		Vec3 spriteBottomLeft = Vec3(SCREEN_CAMERA_CENTER_X - 80.0f, SCREEN_CAMERA_SIZE_Y * 0.125f, 0.0f);
		Vec3 spriteBottomRight = Vec3(SCREEN_CAMERA_CENTER_X + 80.0f, SCREEN_CAMERA_SIZE_Y * 0.125f, 0.0f);
		Vec3 spriteTopLeft = Vec3(SCREEN_CAMERA_CENTER_X - 80.0f, SCREEN_CAMERA_SIZE_Y * 0.375f, 0.0f);
		Vec3 spriteTopRight = Vec3(SCREEN_CAMERA_CENTER_X + 80.0f, SCREEN_CAMERA_SIZE_Y * 0.375f, 0.0f);

		std::vector<Vertex_PNCU> weaponVerts;
		AddVertsForQuad3D(weaponVerts, spriteBottomLeft, spriteBottomRight, spriteTopLeft, spriteTopRight, Rgba8(), spriteUVs);

		g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
		g_theRenderer->SetModelConstants();
		g_theRenderer->BindShader(weaponDef->m_weaponAnimDefs[weaponAnimIndex].m_shader);
		g_theRenderer->BindTexture(&weaponDef->m_weaponAnimDefs[weaponAnimIndex].m_spriteSheet->GetTexture());
		g_theRenderer->DrawVertexArray(static_cast<int>(weaponVerts.size()), weaponVerts.data());
	}

	//render HUD base
	std::vector<Vertex_PNCU> hudVerts;
	AddVertsForQuad3D(hudVerts, Vec3(), Vec3(SCREEN_CAMERA_SIZE_X, 0.0f, 0.0f), Vec3(0.0f, SCREEN_CAMERA_SIZE_Y * 0.125f, 0.0f), Vec3(SCREEN_CAMERA_SIZE_X, SCREEN_CAMERA_SIZE_Y * 0.125f, 0.0f));
	
	g_theRenderer->BindShader(weaponDef->m_hudShader);
	g_theRenderer->BindTexture(weaponDef->m_baseTexture);
	g_theRenderer->DrawVertexArray(static_cast<int>(hudVerts.size()), hudVerts.data());

	//render reticle
	if (m_map->m_currentPlayerActors[m_playerIndex]->m_health > 0)
	{
		std::vector<Vertex_PNCU> reticleVerts;
		Vec3 screenCenter = Vec3(SCREEN_CAMERA_CENTER_X, SCREEN_CAMERA_CENTER_Y, 0.0f);
		//I know this isn't accurate, just tried to make it look the same as in the demo
		float reticleSizeX = weaponDef->m_spriteSize.x / SCREEN_CAMERA_SIZE_X * 48.0f;
		float reticleSizeY = weaponDef->m_spriteSize.y / SCREEN_CAMERA_SIZE_X * 48.0f;
		AddVertsForQuad3D(reticleVerts, screenCenter - Vec3(reticleSizeX, reticleSizeY, 0.0f), screenCenter + Vec3(reticleSizeX, -reticleSizeY, 0.0f), screenCenter + Vec3(-reticleSizeX, reticleSizeY,
			0.0f), screenCenter + Vec3(reticleSizeX, reticleSizeY, 0.0f));

		g_theRenderer->BindTexture(weaponDef->m_reticleTexture);
		g_theRenderer->DrawVertexArray(static_cast<int>(reticleVerts.size()), reticleVerts.data());
	}

	std::string healthText = Stringf("%i", m_map->m_currentPlayerActors[m_playerIndex]->m_health);
	std::vector<Vertex_PCU> textVerts;
	m_map->m_owner->m_menuFont->AddVertsForTextInBox2D(textVerts, AABB2(SCREEN_CAMERA_CENTER_X * 0.60f, 30.0f, SCREEN_CAMERA_CENTER_X * 0.62f, 31.0f), 45.0f, healthText, Rgba8(), 1.0f, 
		Vec2(0.5f, 0.0f), TextBoxMode::OVERRUN);

	std::string killsText = Stringf("%i\nKills", m_numPlayerKills);
	m_map->m_owner->m_menuFont->AddVertsForTextInBox2D(textVerts, AABB2(SCREEN_CAMERA_CENTER_X * 0.13f, 5.0f, SCREEN_CAMERA_CENTER_X * 0.14f, 6.0f), 40.0f, killsText, Rgba8(), 0.7f,
		Vec2(0.5f, 0.0f), TextBoxMode::OVERRUN);
	std::string deathsText = Stringf("%i\nDeaths", m_numPlayerDeaths);
	m_map->m_owner->m_menuFont->AddVertsForTextInBox2D(textVerts, AABB2(SCREEN_CAMERA_SIZE_X - SCREEN_CAMERA_CENTER_X * 0.13f, 5.0f, SCREEN_CAMERA_SIZE_X - SCREEN_CAMERA_CENTER_X * 0.14f, 
		6.0f), 40.0f, deathsText, Rgba8(), 0.7f, Vec2(0.5f, 0.0f), TextBoxMode::OVERRUN);

	g_theRenderer->BindShader(nullptr);
	g_theRenderer->BindTexture(&m_map->m_owner->m_menuFont->GetTexture());
	g_theRenderer->DrawVertexArray(static_cast<int>(textVerts.size()), textVerts.data());

	if (m_map->m_currentPlayerActors[m_playerIndex]->m_health <= 0)
	{
		std::vector<Vertex_PCU> overlayVerts;
		AddVertsForAABB2(overlayVerts, AABB2(0.0f, 0.0f, SCREEN_CAMERA_SIZE_X, SCREEN_CAMERA_SIZE_Y), Rgba8(25, 25, 25, 150));
		g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
		g_theRenderer->SetModelConstants();
		g_theRenderer->BindShader(nullptr);
		g_theRenderer->BindTexture(nullptr);
		g_theRenderer->DrawVertexArray(static_cast<int>(overlayVerts.size()), overlayVerts.data());
	}

	g_theRenderer->EndCamera(m_playerScreenCamera);
}


//
//public player utilities
//
Mat44 Player::GetModelMatrix() const
{
	Mat44 modelMatrix = m_orientation.GetAsMatrix_XFwd_YLeft_ZUp();

	modelMatrix.SetTranslation3D(m_position);

	return modelMatrix;
}
