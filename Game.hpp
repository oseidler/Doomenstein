#pragma once
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/Clock.hpp"


//forward declarations
class Player;
class Map;
class BitmapFont;


//game state enum
enum class GameState
{
	NONE,
	ATTRACT,
	MODE_SELECT,
	LOBBY,
	PLAYING,
	COUNT
};


class Game 
{
//public member functions
public:
	//game flow functions
	void Startup();
	void Update();
	void Render() const;
	void Shutdown();

//public member variables
public:
	//game state vars
	bool		m_isFinished = false;
	GameState	m_currentState = GameState::ATTRACT;
	GameState	m_desiredState = GameState::ATTRACT;
	int			m_keyboardPlayer = -1;
	int			m_controllerPlayer = -1;
	bool		m_isLightsOutMode = false;

	//sounds
	SoundID m_mainMenuMusic;
	SoundPlaybackID m_mainMenuMusicPlayback;
	SoundID m_gameMusic;
	SoundID m_lightsOutMusic;
	SoundPlaybackID m_gameMusicPlayback;
	SoundID m_buttonClickSound;

	//game clock
	Clock m_gameClock = Clock();

	//maps
	Map* m_currentMap;

	//fonts
	BitmapFont* m_menuFont;

//private member functions
private:
	//game flow sub-functions
	void UpdateAttract();
	void RenderAttract() const;

	void UpdateModeSelect();
	void RenderModeSelect() const;

	void UpdateLobby();
	void RenderLobby() const;

	void UpdateGameplay();
	void RenderGameplay() const;

	//mode-switching functions
	void EnterState(GameState state);
	void EnterAttract();
	void ExitAttract();
	void EnterModeSelect();
	void ExitModeSelect();
	void EnterLobby();
	void ExitLobby();
	void EnterGameplay();
	void ExitGameplay();

	//asset management functions
	void LoadAssets();
	void LoadSounds();
	void LoadTextures();
	void LoadDefinitions();

//private member variables
private:
	//camera variables
	Camera m_gameScreenCamera;
};
