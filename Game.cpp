#include "Game/Game.hpp"
#include "Game/Player.hpp"
#include "Game/App.hpp"
#include "Game/TileDefinition.hpp"
#include "Game/MapDefinition.hpp"
#include "Game/ActorDefinition.hpp"
#include "Game/WeaponDefinition.hpp"
#include "Game/Map.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"


//game flow functions
void Game::Startup()
{
	//load game assets and data
	LoadAssets();
	LoadDefinitions();

	//set camera bounds
	m_gameScreenCamera.SetOrthoView(Vec2(0.f, 0.f), Vec2(SCREEN_CAMERA_SIZE_X, SCREEN_CAMERA_SIZE_Y));

	m_menuFont = g_theRenderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont");
	
	EnterState(GameState::ATTRACT);
}


void Game::Update()
{
	if (m_desiredState != m_currentState)
	{
		EnterState(m_desiredState);
	}
	
	switch (m_currentState)
	{
		case GameState::ATTRACT:	 UpdateAttract();	 break;
		case GameState::MODE_SELECT: UpdateModeSelect(); break;
		case GameState::LOBBY:		 UpdateLobby();		 break;
		case GameState::PLAYING:	 UpdateGameplay();	 break;
	}
}


void Game::Render() const
{
	switch (m_currentState)
	{
		case GameState::ATTRACT:	 RenderAttract();	 break;
		case GameState::MODE_SELECT: RenderModeSelect(); break;
		case GameState::LOBBY:		 RenderLobby();		 break;
		case GameState::PLAYING:	 RenderGameplay();	 break;
	}
}


void Game::Shutdown()
{
	//delete all allocated pointers here
	if (m_currentMap != nullptr)
	{
		m_currentMap->Shutdown();

		delete m_currentMap;
		m_currentMap = nullptr;
	}

	//stop any currently playing game music
	g_theAudio->StopSound(m_gameMusicPlayback);
	g_theAudio->StopSound(m_mainMenuMusicPlayback);
}


//
//game flow sub-functions
//
void Game::UpdateAttract()
{
	XboxController const& controller = g_theInput->GetController(0);

	if (g_theInput->WasKeyJustPressed(KEYCODE_ESC) || controller.WasButtonJustPressed(XBOX_BUTTON_SELECT))
	{
		g_theApp->HandleQuitRequested();
	}

	if (g_theInput->WasKeyJustPressed(' ') || controller.WasButtonJustPressed(XBOX_BUTTON_START))
	{
		g_theAudio->StartSound(m_buttonClickSound);
		m_desiredState = GameState::MODE_SELECT;
		m_keyboardPlayer = 0;
	}
}


void Game::RenderAttract() const
{
	g_theRenderer->ClearScreen(Rgba8(100, 100, 100));
	
	g_theRenderer->BeginCamera(m_gameScreenCamera);	//render attract screen with the screen camera

	std::vector<Vertex_PCU> textVerts;
	m_menuFont->AddVertsForTextInBox2D(textVerts, AABB2(SCREEN_CAMERA_CENTER_X - 1.0f, SCREEN_CAMERA_CENTER_Y - 1.0f, SCREEN_CAMERA_CENTER_X + 1.0f, SCREEN_CAMERA_CENTER_Y + 1.0f), 
		100.0f, "Doomenstein", Rgba8(), 1.0f, Vec2(0.5f, 0.5f), TextBoxMode::OVERRUN);
	m_menuFont->AddVertsForTextInBox2D(textVerts, AABB2(SCREEN_CAMERA_CENTER_X - 1.0f, 0.0f, SCREEN_CAMERA_CENTER_X + 1.0f, 1.0f), 25.0f,
		"SPACE or START to begin\nESC or BACK to exit game\n", Rgba8(), 1.0f, Vec2(0.5f, 0.0f), TextBoxMode::OVERRUN);
	
	g_theRenderer->BindShader(nullptr);
	g_theRenderer->BindTexture(&m_menuFont->GetTexture());
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_NONE);
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray(static_cast<int>(textVerts.size()), textVerts.data());
	
	g_theRenderer->EndCamera(m_gameScreenCamera);
}


void Game::UpdateModeSelect()
{
	XboxController const& controller = g_theInput->GetController(0);

	if (g_theInput->WasKeyJustPressed(KEYCODE_LEFT) || g_theInput->WasKeyJustPressed(KEYCODE_RIGHT) || controller.WasButtonJustPressed(XBOX_BUTTON_LEFT) || controller.WasButtonJustPressed(XBOX_BUTTON_RIGHT))
	{
		m_isLightsOutMode = !m_isLightsOutMode;
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_ESC) || controller.WasButtonJustPressed(XBOX_BUTTON_SELECT))
	{
		g_theAudio->StartSound(m_buttonClickSound);
		m_desiredState = GameState::ATTRACT;
	}

	if (m_isLightsOutMode)
	{
		if (g_theInput->WasKeyJustPressed(' '))
		{
			g_theAudio->StartSound(m_buttonClickSound);
			m_desiredState = GameState::PLAYING;
			m_keyboardPlayer = 0;
		}

		if (controller.WasButtonJustPressed(XBOX_BUTTON_START))
		{
			g_theAudio->StartSound(m_buttonClickSound);
			m_desiredState = GameState::PLAYING;
			m_controllerPlayer = 0;
		}
	}
	else
	{
		if (g_theInput->WasKeyJustPressed(' '))
		{
			g_theAudio->StartSound(m_buttonClickSound);
			m_desiredState = GameState::LOBBY;
			m_keyboardPlayer = 0;
		}

		if (controller.WasButtonJustPressed(XBOX_BUTTON_START))
		{
			g_theAudio->StartSound(m_buttonClickSound);
			m_desiredState = GameState::LOBBY;
			m_controllerPlayer = 0;
		}
	}
}


void Game::RenderModeSelect() const
{
	g_theRenderer->ClearScreen(Rgba8(100, 100, 100));

	g_theRenderer->BeginCamera(m_gameScreenCamera);	//render attract screen with the screen camera

	std::vector<Vertex_PCU> textVerts;
	if (m_isLightsOutMode)
	{
		m_menuFont->AddVertsForTextInBox2D(textVerts, AABB2(SCREEN_CAMERA_CENTER_X - 1.0f, SCREEN_CAMERA_CENTER_Y - 1.0f, SCREEN_CAMERA_CENTER_X + 1.0f, SCREEN_CAMERA_CENTER_Y + 1.0f),
			80.0f, "Mode: Lights Out", Rgba8(), 1.0f, Vec2(0.5f, 0.5f), TextBoxMode::OVERRUN);
		m_menuFont->AddVertsForTextInBox2D(textVerts, AABB2(SCREEN_CAMERA_CENTER_X - 1.0f, SCREEN_CAMERA_CENTER_Y - 101.0f, SCREEN_CAMERA_CENTER_X + 1.0f, SCREEN_CAMERA_CENTER_Y - 100.0f),
			60.0f, "1 player", Rgba8(), 1.0f, Vec2(0.5f, 0.5f), TextBoxMode::OVERRUN);
	}
	else
	{
		m_menuFont->AddVertsForTextInBox2D(textVerts, AABB2(SCREEN_CAMERA_CENTER_X - 1.0f, SCREEN_CAMERA_CENTER_Y - 1.0f, SCREEN_CAMERA_CENTER_X + 1.0f, SCREEN_CAMERA_CENTER_Y + 1.0f),
			80.0f, "Mode: Standard", Rgba8(), 1.0f, Vec2(0.5f, 0.5f), TextBoxMode::OVERRUN);
		m_menuFont->AddVertsForTextInBox2D(textVerts, AABB2(SCREEN_CAMERA_CENTER_X - 1.0f, SCREEN_CAMERA_CENTER_Y - 101.0f, SCREEN_CAMERA_CENTER_X + 1.0f, SCREEN_CAMERA_CENTER_Y - 100.0f),
			60.0f, "1-2 players", Rgba8(), 1.0f, Vec2(0.5f, 0.5f), TextBoxMode::OVERRUN);
	}
	
	m_menuFont->AddVertsForTextInBox2D(textVerts, AABB2(SCREEN_CAMERA_CENTER_X - 1.0f, 0.0f, SCREEN_CAMERA_CENTER_X + 1.0f, 1.0f), 25.0f,
		"Arrow keys or D-pad to choose mode\nSPACE to join with keyboard\nSTART to join with controller\n", Rgba8(), 1.0f, Vec2(0.5f, 0.0f), TextBoxMode::OVERRUN);

	g_theRenderer->BindShader(nullptr);
	g_theRenderer->BindTexture(&m_menuFont->GetTexture());
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_NONE);
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray(static_cast<int>(textVerts.size()), textVerts.data());

	g_theRenderer->EndCamera(m_gameScreenCamera);
}


void Game::UpdateLobby()
{
	XboxController const& controller = g_theInput->GetController(0);

	if (g_theInput->WasKeyJustPressed(KEYCODE_ESC))
	{
		g_theAudio->StartSound(m_buttonClickSound);
		
		if (m_keyboardPlayer != -1 && m_controllerPlayer != -1)
		{
			m_keyboardPlayer = -1;
		}
		else
		{
			m_desiredState = GameState::ATTRACT;
		}
	}

	if (controller.WasButtonJustPressed(XBOX_BUTTON_SELECT))
	{
		g_theAudio->StartSound(m_buttonClickSound);

		if (m_controllerPlayer != -1 && m_keyboardPlayer != -1)
		{
			m_controllerPlayer = -1;
		}
		else
		{
			m_desiredState = GameState::ATTRACT;
		}
	}

	if (g_theInput->WasKeyJustPressed(' '))
	{
		g_theAudio->StartSound(m_buttonClickSound);

		if (m_keyboardPlayer == -1)
		{
			if (m_controllerPlayer == 0)
			{
				m_keyboardPlayer = 1;
			}
			else
			{
				m_keyboardPlayer = 0;
			}
		}
		else
		{
			m_desiredState = GameState::PLAYING;
		}
	}

	if (controller.WasButtonJustPressed(XBOX_BUTTON_START))
	{
		g_theAudio->StartSound(m_buttonClickSound);
		
		if (m_controllerPlayer == -1)
		{
			if (m_keyboardPlayer == 0)
			{
				m_controllerPlayer = 1;
			}
			else
			{
				m_controllerPlayer = 0;
			}
		}
		else
		{
			m_desiredState = GameState::PLAYING;
		}
	}

	if (m_keyboardPlayer == -1 && m_controllerPlayer == 1)
	{
		m_controllerPlayer = 0;
	}
	else if (m_keyboardPlayer == 1 && m_controllerPlayer == -1)
	{
		m_keyboardPlayer = 0;
	}
}


void Game::RenderLobby() const
{
	g_theRenderer->ClearScreen(Rgba8(100, 100, 100));

	g_theRenderer->BeginCamera(m_gameScreenCamera);

	g_theRenderer->BindShader(nullptr);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_NONE);
	g_theRenderer->SetModelConstants();
	
	std::vector<Vertex_PCU> textVerts;

	if (m_keyboardPlayer == 0)
	{
		if (m_controllerPlayer == -1)
		{
			m_menuFont->AddVertsForTextInBox2D(textVerts, AABB2(SCREEN_CAMERA_CENTER_X - 1.0f, SCREEN_CAMERA_CENTER_Y - 1.0f, SCREEN_CAMERA_CENTER_X + 1.0f, SCREEN_CAMERA_CENTER_Y + 1.0f),
				45.0f, "Player 1\nMouse And Keyboard\n", Rgba8(), 0.8f, Vec2(0.5f, 0.5f), TextBoxMode::OVERRUN);
			m_menuFont->AddVertsForTextInBox2D(textVerts, AABB2(SCREEN_CAMERA_CENTER_X - 1.0f, SCREEN_CAMERA_CENTER_Y * 0.85f - 1.0f, SCREEN_CAMERA_CENTER_X + 1.0f, SCREEN_CAMERA_CENTER_Y * 0.85f + 1.0f),
				20.0f, "SPACE to start game\nSTART to join player\nESC to leave game", Rgba8(), 0.8f, Vec2(0.5f, 0.5f), TextBoxMode::OVERRUN);
		}
		else
		{
			m_menuFont->AddVertsForTextInBox2D(textVerts, AABB2(SCREEN_CAMERA_CENTER_X - 1.0f, SCREEN_CAMERA_CENTER_Y * 1.5f - 1.0f, SCREEN_CAMERA_CENTER_X + 1.0f, SCREEN_CAMERA_CENTER_Y * 1.5f + 1.0f),
				45.0f, "Player 1\nMouse And Keyboard\n", Rgba8(), 0.8f, Vec2(0.5f, 0.5f), TextBoxMode::OVERRUN);
			m_menuFont->AddVertsForTextInBox2D(textVerts, AABB2(SCREEN_CAMERA_CENTER_X - 1.0f, SCREEN_CAMERA_CENTER_Y * 1.35f - 1.0f, SCREEN_CAMERA_CENTER_X + 1.0f, SCREEN_CAMERA_CENTER_Y * 1.35f + 1.0f),
				20.0f, "SPACE to start game\nESC to leave game", Rgba8(), 0.8f, Vec2(0.5f, 0.5f), TextBoxMode::OVERRUN);
			m_menuFont->AddVertsForTextInBox2D(textVerts, AABB2(SCREEN_CAMERA_CENTER_X - 1.0f, SCREEN_CAMERA_CENTER_Y * 0.5f - 1.0f, SCREEN_CAMERA_CENTER_X + 1.0f, SCREEN_CAMERA_CENTER_Y * 0.5f + 1.0f),
				45.0f, "Player 2\nController\n", Rgba8(), 0.8f, Vec2(0.5f, 0.5f), TextBoxMode::OVERRUN);
			m_menuFont->AddVertsForTextInBox2D(textVerts, AABB2(SCREEN_CAMERA_CENTER_X - 1.0f, SCREEN_CAMERA_CENTER_Y * 0.35f - 1.0f, SCREEN_CAMERA_CENTER_X + 1.0f, SCREEN_CAMERA_CENTER_Y * 0.35f + 1.0f),
				20.0f, "START to start game\nBACK to leave game", Rgba8(), 0.8f, Vec2(0.5f, 0.5f), TextBoxMode::OVERRUN);
		}
	}
	else if (m_controllerPlayer == 0)
	{
		if (m_keyboardPlayer == -1)
		{
			m_menuFont->AddVertsForTextInBox2D(textVerts, AABB2(SCREEN_CAMERA_CENTER_X - 1.0f, SCREEN_CAMERA_CENTER_Y - 1.0f, SCREEN_CAMERA_CENTER_X + 1.0f, SCREEN_CAMERA_CENTER_Y + 1.0f),
				45.0f, "Player 1\nController\n", Rgba8(), 0.8f, Vec2(0.5f, 0.5f), TextBoxMode::OVERRUN);
			m_menuFont->AddVertsForTextInBox2D(textVerts, AABB2(SCREEN_CAMERA_CENTER_X - 1.0f, SCREEN_CAMERA_CENTER_Y * 0.85f - 1.0f, SCREEN_CAMERA_CENTER_X + 1.0f, SCREEN_CAMERA_CENTER_Y * 0.85f + 1.0f),
				20.0f, "START to start game\nSPACE to join player\nBACK to leave game", Rgba8(), 0.8f, Vec2(0.5f, 0.5f), TextBoxMode::OVERRUN);
		}
		else
		{
			m_menuFont->AddVertsForTextInBox2D(textVerts, AABB2(SCREEN_CAMERA_CENTER_X - 1.0f, SCREEN_CAMERA_CENTER_Y * 1.5f - 1.0f, SCREEN_CAMERA_CENTER_X + 1.0f, SCREEN_CAMERA_CENTER_Y * 1.5f + 1.0f),
				45.0f, "Player 1\nController\n", Rgba8(), 0.8f, Vec2(0.5f, 0.5f), TextBoxMode::OVERRUN);
			m_menuFont->AddVertsForTextInBox2D(textVerts, AABB2(SCREEN_CAMERA_CENTER_X - 1.0f, SCREEN_CAMERA_CENTER_Y * 1.35f - 1.0f, SCREEN_CAMERA_CENTER_X + 1.0f, SCREEN_CAMERA_CENTER_Y * 1.35f + 1.0f),
				20.0f, "START to start game\nBACK to leave game", Rgba8(), 0.8f, Vec2(0.5f, 0.5f), TextBoxMode::OVERRUN);
			m_menuFont->AddVertsForTextInBox2D(textVerts, AABB2(SCREEN_CAMERA_CENTER_X - 1.0f, SCREEN_CAMERA_CENTER_Y * 0.5f - 1.0f, SCREEN_CAMERA_CENTER_X + 1.0f, SCREEN_CAMERA_CENTER_Y * 0.5f + 1.0f),
				45.0f, "Player 2\nKeyboard\n", Rgba8(), 0.8f, Vec2(0.5f, 0.5f), TextBoxMode::OVERRUN);
			m_menuFont->AddVertsForTextInBox2D(textVerts, AABB2(SCREEN_CAMERA_CENTER_X - 1.0f, SCREEN_CAMERA_CENTER_Y * 0.35f - 1.0f, SCREEN_CAMERA_CENTER_X + 1.0f, SCREEN_CAMERA_CENTER_Y * 0.35f + 1.0f),
				20.0f, "SPACE to start game\nESC to leave game", Rgba8(), 0.8f, Vec2(0.5f, 0.5f), TextBoxMode::OVERRUN);
		}
	}

	g_theRenderer->BindShader(nullptr);
	g_theRenderer->BindTexture(&m_menuFont->GetTexture());
	g_theRenderer->DrawVertexArray(static_cast<int>(textVerts.size()), textVerts.data());

	g_theRenderer->EndCamera(m_gameScreenCamera);
}


void Game::UpdateGameplay()
{
	XboxController const& controller = g_theInput->GetController(0);
	
	//check if p is pressed to toggle pause
	if (g_theInput->WasKeyJustPressed('P'))
	{
		m_gameClock.TogglePause();
	}

	//check if t is currently being held to turn slow-mo on
	if (g_theInput->IsKeyDown('T'))
	{
		m_gameClock.SetTimeScale(0.1f);
	}
	else
	{
		m_gameClock.SetTimeScale(1.0f);
	}

	//check if o is pressed and do single step if so
	if (g_theInput->WasKeyJustPressed('O'))
	{
		m_gameClock.StepSingleFrame();
	}

	//update player
	float deltaSeconds = m_gameClock.GetDeltaSeconds();

	m_currentMap->Update(deltaSeconds);

	if (g_theInput->WasKeyJustPressed(KEYCODE_ESC) || controller.WasButtonJustPressed(XBOX_BUTTON_SELECT))
	{
		g_theApp->RestartGame();
	}
}


void Game::RenderGameplay() const
{
	g_theRenderer->ClearScreen(Rgba8(50, 50, 50));	//clear screen to dark gray

	g_theRenderer->BeginCamera(m_currentMap->m_players[0]->m_playerCamera);	//render game world with the world camera

	//game renderering here
	m_currentMap->Render(0);

	g_theRenderer->EndCamera(m_currentMap->m_players[0]->m_playerCamera);

	if (m_currentMap->m_players[1] != nullptr)
	{
		g_theRenderer->BeginCamera(m_currentMap->m_players[1]->m_playerCamera);	//render game world with the world camera

		//game renderering here
		m_currentMap->Render(1);

		g_theRenderer->EndCamera(m_currentMap->m_players[1]->m_playerCamera);
	}

	//debug world rendering
	DebugRenderWorld(m_currentMap->m_players[0]->m_playerCamera);
	if (m_currentMap->m_players[1] != nullptr)
	{
		DebugRenderWorld(m_currentMap->m_players[1]->m_playerCamera);
	}

	//g_theRenderer->BeginCamera(m_gameScreenCamera);	//render UI with the screen camera

	//screen camera rendering here
	if (m_currentMap->m_players[0] != nullptr)
	{
		m_currentMap->m_players[0]->RenderHUD();
	}
	if (m_currentMap->m_players[1] != nullptr)
	{
		m_currentMap->m_players[1]->RenderHUD();
	}

	//g_theRenderer->EndCamera(m_gameScreenCamera);

	//debug screen rendering
	DebugRenderScreen(m_gameScreenCamera);
}


//
//mode switching functions
//
void Game::EnterState(GameState state)
{
	switch (m_currentState)
	{
		case GameState::ATTRACT:	 ExitAttract();    break;
		case GameState::MODE_SELECT: ExitModeSelect(); break;
		case GameState::LOBBY:		 ExitLobby();	   break;
		case GameState::PLAYING:	 ExitGameplay();   break;
	}
	
	switch (state)
	{
		case GameState::ATTRACT:	 EnterAttract();    break;
		case GameState::MODE_SELECT: EnterModeSelect(); break;
		case GameState::LOBBY:		 EnterLobby();	    break;
		case GameState::PLAYING:	 EnterGameplay();   break;
	}

	m_currentState = state;
}


void Game::EnterAttract()
{
	//reset players
	m_keyboardPlayer = -1;
	m_controllerPlayer = -1;
	
	//start music
	if (!g_theAudio->IsPlaying(m_mainMenuMusicPlayback))
	{
		m_mainMenuMusicPlayback = g_theAudio->StartSound(m_mainMenuMusic, true, g_gameConfigBlackboard.GetValue("musicVolume", 1.0f));
	}
}


void Game::ExitAttract()
{
}


void Game::EnterModeSelect()
{
}


void Game::ExitModeSelect()
{
}


void Game::EnterLobby()
{
}


void Game::ExitLobby()
{
}


void Game::EnterGameplay()
{
	//change music
	g_theAudio->StopSound(m_mainMenuMusicPlayback);
	std::string mapType = "defaultMap";
	if (!m_isLightsOutMode)
	{
		m_gameMusicPlayback = g_theAudio->StartSound(m_gameMusic, true, g_gameConfigBlackboard.GetValue("musicVolume", 0.1f));
	}
	else
	{
		m_gameMusicPlayback = g_theAudio->StartSound(m_lightsOutMusic, true, g_gameConfigBlackboard.GetValue("lightsOutMusicVolume", 0.25f));
		mapType = "lightsOutMap";
	}

	//create map
	if (m_keyboardPlayer == -1)
	{
		std::string mapName = g_gameConfigBlackboard.GetValue(mapType, "testMap");
		m_currentMap = new Map(this, MapDefinition::GetMapDefinition(mapName), 1, 0, -1);
		m_currentMap->Startup();
		g_theAudio->SetNumListeners(1);
	}
	else if (m_controllerPlayer == -1)
	{
		std::string mapName = g_gameConfigBlackboard.GetValue(mapType, "testMap");
		m_currentMap = new Map(this, MapDefinition::GetMapDefinition(mapName), 1, -1, -1);
		m_currentMap->Startup();
		g_theAudio->SetNumListeners(1);
	}
	else if (m_keyboardPlayer == 0 && m_controllerPlayer == 1)
	{
		std::string mapName = g_gameConfigBlackboard.GetValue(mapType, "testMap");
		m_currentMap = new Map(this, MapDefinition::GetMapDefinition(mapName), 2, -1, 0);
		m_currentMap->Startup();
		g_theAudio->SetNumListeners(2);

	}
	else if (m_controllerPlayer == 0 && m_keyboardPlayer == 1)
	{
		std::string mapName = g_gameConfigBlackboard.GetValue(mapType, "testMap");
		m_currentMap = new Map(this, MapDefinition::GetMapDefinition(mapName), 2, 0, -1);
		m_currentMap->Startup();
		g_theAudio->SetNumListeners(2);
	}
}


void Game::ExitGameplay()
{
	//destroy map
	if (m_currentMap != nullptr)
	{
		m_currentMap->Shutdown();

		delete m_currentMap;
		m_currentMap = nullptr;
	}
}


//
//asset management functions
//
void Game::LoadAssets()
{
	LoadSounds();
	LoadTextures();
}


void Game::LoadSounds()
{
	m_mainMenuMusic = g_theAudio->CreateOrGetSound(g_gameConfigBlackboard.GetValue("mainMenuMusic", ""));
	m_gameMusic = g_theAudio->CreateOrGetSound(g_gameConfigBlackboard.GetValue("gameMusic", ""));
	m_lightsOutMusic = g_theAudio->CreateOrGetSound(g_gameConfigBlackboard.GetValue("lightsOutMusic", ""));
	m_buttonClickSound = g_theAudio->CreateOrGetSound(g_gameConfigBlackboard.GetValue("buttonClickSound", ""));
}


void Game::LoadTextures()
{
	 
}


void Game::LoadDefinitions()
{
	if (TileDefinition::s_tileDefinitions.size() == 0)
	{
		TileDefinition::InitializeTileDefs();
	}
	if (MapDefinition::s_mapDefinitions.size() == 0)
	{
		MapDefinition::InitializeMapDefs();
	}
	if (ActorDefinition::s_actorDefinitions.size() == 0)
	{
		ActorDefinition::InitializeActorDefs();
	}
	if (ActorDefinition::s_projectileActorDefinitions.size() == 0)
	{
		ActorDefinition::InitializeProjectileActorDefs();
	}
	if (WeaponDefinition::s_weaponDefinitions.size() == 0)
	{
		WeaponDefinition::InitializeWeaponDefs();
	}
}
