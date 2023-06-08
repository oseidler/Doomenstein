#include "Game/Map.hpp"
#include "Game/Game.hpp"
#include "Game/TileDefinition.hpp"
#include "Game/MapDefinition.hpp"
#include "Game/ActorDefinition.hpp"
#include "Game/Actor.hpp"
#include "Game/Player.hpp"
#include "Game/AI.hpp"
#include "Game/Weapon.hpp"
#include "Game/WeaponDefinition.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Renderer/IndexBuffer.hpp"
#include "Engine/Renderer/ConstantBuffer.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Window/Window.hpp"


//special flashlight constants for lights out mode
struct FlashlightConstants
{
	Vec3  FlashlightPosition;
	float FlashlightIntensity;
	float FlashlightSize;
	Vec3  FlashlightAtt;
};
static const int k_lightConstantsSlot = 4;


//
//constructor
//
Map::Map(Game* owner, MapDefinition const* definition, int numPlayers, int player1XboxID, int player2XboxID)
	: m_owner(owner)
	, m_definition(definition)
	, m_numPlayers(numPlayers)
{
	m_tileVertBuffer = g_theRenderer->CreateVertexBuffer(sizeof(Vertex_PNCU), sizeof(Vertex_PNCU));
	m_tileIndexBuffer = g_theRenderer->CreateIndexBuffer(sizeof(unsigned int));
	m_flashlightConstants = g_theRenderer->CreateConstantBuffer(sizeof(FlashlightConstants));

	m_tileSpriteSheet = new SpriteSheet(*m_definition->m_spriteSheetTexture, m_definition->m_spriteSheetCellCount);

	TileDefinition const* stoneFloor = TileDefinition::GetTileDefinition("StoneFloor");
	TileDefinition const* woodFloor = TileDefinition::GetTileDefinition("WoodFloor");
	TileDefinition const* dirtFloor = TileDefinition::GetTileDefinition("OpenDirt");
	TileDefinition const* grassFloor = TileDefinition::GetTileDefinition("OpenGrass");
	TileDefinition const* brickWall = TileDefinition::GetTileDefinition("BrickWall");
	TileDefinition const* woodWall = TileDefinition::GetTileDefinition("WoodWall");
	TileDefinition const* stoneWall = TileDefinition::GetTileDefinition("StoneWall");
	TileDefinition const* crackedStoneWall = TileDefinition::GetTileDefinition("CrackedStoneWall");
	TileDefinition const* stoneFloor2 = TileDefinition::GetTileDefinition("StoneFloor2");

	unsigned char const* imageData = (unsigned char*)m_definition->m_image.GetRawData();
	m_dimensions = m_definition->m_image.GetImageDimensions();

	int tileXCounter = 0;
	int tileYCounter = 0;

	for (int byteIndex = 0; byteIndex < (m_dimensions.x * m_dimensions.y * 4); byteIndex += 4)
	{
		IntVec2 tileCoords = IntVec2(tileXCounter, tileYCounter);

		Rgba8 pixelColor = Rgba8(imageData[byteIndex], imageData[byteIndex + 1], imageData[byteIndex + 2], imageData[byteIndex + 3]);

		//yes, I know this is a dumb way to do it
		if (pixelColor == stoneFloor->m_mapImagePixelColor)
		{
			m_tiles.push_back(Tile(tileCoords, stoneFloor));
		}
		else if (pixelColor == woodFloor->m_mapImagePixelColor)
		{
			m_tiles.push_back(Tile(tileCoords, woodFloor));
		}
		else if (pixelColor == dirtFloor->m_mapImagePixelColor)
		{
			m_tiles.push_back(Tile(tileCoords, dirtFloor));
		}
		else if (pixelColor == grassFloor->m_mapImagePixelColor)
		{
			m_tiles.push_back(Tile(tileCoords, grassFloor));
		}
		else if (pixelColor == brickWall->m_mapImagePixelColor)
		{
			m_tiles.push_back(Tile(tileCoords, brickWall));
		}
		else if (pixelColor == woodWall->m_mapImagePixelColor)
		{
			m_tiles.push_back(Tile(tileCoords, woodWall));
		}
		else if (pixelColor == stoneWall->m_mapImagePixelColor)
		{
			m_tiles.push_back(Tile(tileCoords, stoneWall));
		}
		else if (pixelColor == stoneFloor2->m_mapImagePixelColor)
		{
			m_tiles.push_back(Tile(tileCoords, stoneFloor2));
		}
		else if (pixelColor == crackedStoneWall->m_mapImagePixelColor)
		{
			m_tiles.push_back(Tile(tileCoords, crackedStoneWall));
		}

		tileXCounter++;
		if (tileXCounter >= m_dimensions.x)
		{
			tileXCounter = 0;
			tileYCounter++;
		}
	}

	for (int tileIndex = 0; tileIndex < m_tiles.size(); tileIndex++)
	{
		m_tiles[tileIndex].AddVertsForTile(m_tileVerts, m_tileVertIndexes, m_tileSpriteSheet, m_definition->m_spriteSheetCellCount);
	}

	for (int actorIndex = 0; actorIndex < m_definition->m_spawnInfos.size(); actorIndex++)
	{
		SpawnInfo const& spawnInfo = m_definition->m_spawnInfos[actorIndex];
		SpawnActor(spawnInfo.m_actorName, spawnInfo.m_position, spawnInfo.m_orientation, spawnInfo.m_velocity);
	}

	//create players
	m_players.resize(2);
	m_currentPlayerActors.resize(2);

	if (m_numPlayers == 2)
	{
		m_players[0] = new Player(m_owner, this, 0, player1XboxID);

		m_players[0]->m_playerCamera.SetOrthoView(Vec2(WORLD_CAMERA_MIN_X, WORLD_CAMERA_MIN_Y), Vec2(WORLD_CAMERA_MAX_X, WORLD_CAMERA_MAX_Y));
		m_players[0]->m_playerCamera.SetPerspectiveView(4.0f, 60.0f, 0.1f, 100.0f);
		IntVec2 clientDimensions = g_theWindow->GetClientDimensions();
		float screenWidth = static_cast<float>(clientDimensions.x);
		float screenHeight = static_cast<float>(clientDimensions.y) * 0.5f;
		m_players[0]->m_playerCamera.SetViewport(Vec2(0.0f, 0.0f), Vec2(screenWidth, screenHeight));
		m_players[0]->m_playerCamera.SetRenderBasis(Vec3(0.0f, 0.0f, 1.0f), Vec3(-1.0f, 0.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f));

		m_players[0]->m_playerScreenCamera.SetOrthoView(Vec2(0.0f, 0.0f), Vec2(SCREEN_CAMERA_SIZE_X, SCREEN_CAMERA_SIZE_Y));
		m_players[0]->m_playerScreenCamera.SetViewport(Vec2(0.0f, 0.0f), Vec2(screenWidth, screenHeight));

		SpawnPlayer(0);

		m_players[1] = new Player(m_owner, this, 1, player2XboxID);

		m_players[1]->m_playerCamera.SetOrthoView(Vec2(WORLD_CAMERA_MIN_X, WORLD_CAMERA_MIN_Y), Vec2(WORLD_CAMERA_MAX_X, WORLD_CAMERA_MAX_Y));
		m_players[1]->m_playerCamera.SetPerspectiveView(4.0f, 60.0f, 0.1f, 100.0f);
		m_players[1]->m_playerCamera.SetViewport(Vec2(0.0f, screenHeight), Vec2(screenWidth, screenHeight));
		m_players[1]->m_playerCamera.SetRenderBasis(Vec3(0.0f, 0.0f, 1.0f), Vec3(-1.0f, 0.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f));

		m_players[1]->m_playerScreenCamera.SetOrthoView(Vec2(0.0f, 0.0f), Vec2(SCREEN_CAMERA_SIZE_X, SCREEN_CAMERA_SIZE_Y));
		m_players[1]->m_playerScreenCamera.SetViewport(Vec2(0.0f, screenHeight), Vec2(screenWidth, screenHeight));

		SpawnPlayer(1);
	}
	else
	{
		m_players[0] = new Player(m_owner, this, 0, player1XboxID);

		m_players[0]->m_playerCamera.SetOrthoView(Vec2(WORLD_CAMERA_MIN_X, WORLD_CAMERA_MIN_Y), Vec2(WORLD_CAMERA_MAX_X, WORLD_CAMERA_MAX_Y));
		m_players[0]->m_playerCamera.SetPerspectiveView(2.0f, 60.0f, 0.1f, 100.0f);
		m_players[0]->m_playerCamera.SetRenderBasis(Vec3(0.0f, 0.0f, 1.0f), Vec3(-1.0f, 0.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f));

		m_players[0]->m_playerScreenCamera.SetOrthoView(Vec2(0.0f, 0.0f), Vec2(SCREEN_CAMERA_SIZE_X, SCREEN_CAMERA_SIZE_Y));

		SpawnPlayer(0);
	}

	/*for (int actorIndex = 0; actorIndex < m_allActors.size(); actorIndex++)
	{
		Actor*& actor = m_allActors[actorIndex];

		if (actor != nullptr)
		{
			actor->Startup();
		}
	}*/
}


//
//public game flow functions
//
void Map::Startup()
{
}


void Map::Update(float deltaSeconds)
{
	//debug lighting controls
	if (g_theInput->WasKeyJustPressed(KEYCODE_F1))
	{
		std::string sunDirectionText = Stringf("Current sun direction: %.2f, %.2f (F2, F3 to change x; F4, F5 to change y", m_sunDirection.x, m_sunDirection.y);
		DebugAddMessage(sunDirectionText, 4.0f);
		std::string sunIntensityText = Stringf("Current sun intensity: %.2f (F6, F7 to change)", m_sunIntensity);
		DebugAddMessage(sunIntensityText, 4.0f);
		std::string ambientIntensityText = Stringf("Current ambient intensity: %.2f (F6, F7 to change)", m_ambientIntensity);
		DebugAddMessage(ambientIntensityText, 4.0f);
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_F2))
	{
		m_sunDirection.x -= 1.0f;
		std::string sunDirectionText = Stringf("New x sun direction: %.2f", m_sunDirection.x);
		DebugAddMessage(sunDirectionText, 4.0f);
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_F3))
	{
		m_sunDirection.x += 1.0f;
		std::string sunDirectionText = Stringf("New x sun direction: %.2f", m_sunDirection.x);
		DebugAddMessage(sunDirectionText, 4.0f);
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_F4))
	{
		m_sunDirection.y -= 1.0f;
		std::string sunDirectionText = Stringf("New y sun direction: %.2f", m_sunDirection.y);
		DebugAddMessage(sunDirectionText, 4.0f);
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_F5))
	{
		m_sunDirection.y += 1.0f;
		std::string sunDirectionText = Stringf("New y sun direction: %.2f", m_sunDirection.y);
		DebugAddMessage(sunDirectionText, 4.0f);
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_F6))
	{
		m_sunIntensity -= 0.05f;
		m_sunIntensity = GetClamped(m_sunIntensity, 0.0f, 1.0f);
		std::string sunText = Stringf("New sun intensity: %.2f", m_sunIntensity);
		DebugAddMessage(sunText, 4.0f);
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_F7))
	{
		m_sunIntensity += 0.05f;
		m_sunIntensity = GetClamped(m_sunIntensity, 0.0f, 1.0f);
		std::string sunText = Stringf("New sun intensity: %.2f", m_sunIntensity);
		DebugAddMessage(sunText, 4.0f);
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_F8))
	{
		m_ambientIntensity -= 0.05f;
		m_ambientIntensity = GetClamped(m_ambientIntensity, 0.0f, 1.0f);
		std::string ambientText = Stringf("New ambient intensity: %.2f", m_ambientIntensity);
		DebugAddMessage(ambientText, 4.0f);
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_F9))
	{
		m_ambientIntensity += 0.05f;
		m_ambientIntensity = GetClamped(m_ambientIntensity, 0.0f, 1.0f);
		std::string ambientText = Stringf("New ambient intensity: %.2f", m_ambientIntensity);
		DebugAddMessage(ambientText, 4.0f);
	}
	
	if (m_currentPlayerActors[0] == nullptr)
	{
		SpawnPlayer(0);
	}
	if (m_currentPlayerActors[1] == nullptr && m_numPlayers == 2)
	{
		SpawnPlayer(1);
	}
	
	m_players[0]->Update(deltaSeconds);
	if (m_numPlayers == 2)
	{
		m_players[1]->Update(deltaSeconds);
	}
	
	for (int actorIndex = 0; actorIndex < m_allActors.size(); actorIndex++)
	{
		Actor*& actor = m_allActors[actorIndex];

		if (actor != nullptr)
		{
			actor->Update(deltaSeconds);
		}
	}

	CollideAllActorsWithEachOther();
	CollideAllActorsWithMap();

	g_theAudio->UpdateListener(0, m_players[0]->m_position, m_players[0]->GetModelMatrix().GetIBasis3D(), m_players[0]->GetModelMatrix().GetKBasis3D());
	if (m_numPlayers == 2)
	{
		g_theAudio->UpdateListener(1, m_players[1]->m_position, m_players[1]->GetModelMatrix().GetIBasis3D(), m_players[1]->GetModelMatrix().GetKBasis3D());
	}

	DeleteDestroyedActors();
}


void Map::Render(int currentPlayerRendering)
{
	g_theRenderer->BindShader(m_definition->m_shader);
	g_theRenderer->BindTexture(&m_tileSpriteSheet->GetTexture());
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->SetModelConstants();
	
	if (m_owner->m_isLightsOutMode)
	{
		Weapon*& flashlight = m_currentPlayerActors[0]->m_currentWeapon;
		
		g_theRenderer->SetLightConstants(m_sunDirection, m_lightsOutSunIntensity, m_lightsOutAmbientIntensity);
		if (flashlight != nullptr)
		{
			SetFlashlightConstants(Vec3(691.0f, 345.0f, 0.0f), flashlight->m_currentLightIntensity, flashlight->m_currentLightRadius, Vec3(0.0f, 0.2f, 0.0f));
		}
		else
		{
			SetFlashlightConstants(Vec3(), 0.0f, 0.0f, Vec3());
		}
	}
	else
	{
		g_theRenderer->SetLightConstants(m_sunDirection, m_sunIntensity, m_ambientIntensity);
		SetFlashlightConstants(Vec3(), 0.0f, 0.0f, Vec3());
	}

	g_theRenderer->CopyCPUToGPU(m_tileVerts.data(), static_cast<int>(m_tileVerts.size()) * sizeof(Vertex_PNCU), m_tileVertBuffer);
	g_theRenderer->CopyCPUToGPU(m_tileVertIndexes.data(), static_cast<int>(m_tileVertIndexes.size()) * sizeof(unsigned int), m_tileIndexBuffer);
	g_theRenderer->DrawVertexBufferIndexed(m_tileVertBuffer, m_tileIndexBuffer, static_cast<int>(m_tileVertIndexes.size()));

	for (int actorIndex = 0; actorIndex < m_allActors.size(); actorIndex++)
	{
		Actor*& actor = m_allActors[actorIndex];

		if (actor != nullptr)
		{
			actor->Render(currentPlayerRendering);
		}
	}
}


void Map::Shutdown()
{
	if (m_tileSpriteSheet != nullptr)
	{
		delete m_tileSpriteSheet;
		m_tileSpriteSheet = nullptr;
	}

	if (m_tileVertBuffer != nullptr)
	{
		delete m_tileVertBuffer;
		m_tileVertBuffer = nullptr;
	}

	if (m_tileIndexBuffer != nullptr)
	{
		delete m_tileIndexBuffer;
		m_tileIndexBuffer = nullptr;
	}

	if (m_flashlightConstants != nullptr)
	{
		delete m_flashlightConstants;
		m_flashlightConstants = nullptr;
	}

	for (int actorIndex = 0; actorIndex < m_allActors.size(); actorIndex++)
	{
		Actor*& actor = m_allActors[actorIndex];

		if (actor != nullptr)
		{
			delete actor;
			actor = nullptr;
		}
	}

	for (int playerIndex = 0; playerIndex < m_players.size(); playerIndex++)
	{
		if (m_players[playerIndex] != nullptr)
		{
			delete m_players[playerIndex];
			m_players[playerIndex] = nullptr;
		}
	}
}


//
//actor handling functions
//
void Map::DeleteDestroyedActors()
{
	for (int actorIndex = 0; actorIndex < m_allActors.size(); actorIndex++)
	{
		Actor*& actor = m_allActors[actorIndex];
		if (actor != nullptr && actor->m_isGarbage)
		{
			delete actor;
			if (m_currentPlayerActors[0] == actor)
			{
				m_currentPlayerActors[0] = nullptr;
			}
			if (m_currentPlayerActors[1] == actor)
			{
				m_currentPlayerActors[1] = nullptr;
			}
			actor = nullptr;
		}
	}
}


Actor* Map::SpawnPlayer(int playerIndex)
{
	if (playerIndex >= m_players.size())
	{
		return nullptr;
	}

	int spawnPointIndex;

	do 
	{
		spawnPointIndex = g_rng.RollRandomIntInRange(0, static_cast<int>(m_allActors.size() - 1));
	} while (m_allActors[spawnPointIndex] == nullptr || m_allActors[spawnPointIndex]->m_definition->m_name != "SpawnPoint");

	if (m_owner->m_isLightsOutMode)
	{
		m_currentPlayerActors[playerIndex] = SpawnActor("FlashlightMarine", m_allActors[spawnPointIndex]->m_position, m_allActors[spawnPointIndex]->m_orientation);
	}
	else
	{
		m_currentPlayerActors[playerIndex] = SpawnActor("Marine", m_allActors[spawnPointIndex]->m_position, m_allActors[spawnPointIndex]->m_orientation);
	}
	m_players[playerIndex]->Possess(m_currentPlayerActors[playerIndex]->m_UID);
	return m_currentPlayerActors[playerIndex];
}


Actor* Map::SpawnActor(std::string actorDefName, Vec3 const& position, EulerAngles const& orientation, Vec3 const& velocity)
{
	ActorDefinition const* definition = ActorDefinition::GetActorDefinition(actorDefName);

	if (definition != nullptr)
	{
		//loop through actor list to find empty slot
		for (int actorIndex = 0; actorIndex < m_allActors.size(); actorIndex++)
		{
			if (m_allActors[actorIndex] == nullptr)
			{
				ActorUID nextUID = ActorUID(actorIndex, m_actorSalt);
				Actor* newActor = new Actor(nextUID, definition, this, position, orientation, velocity);
				m_allActors[actorIndex] = newActor;
				newActor->Startup();
				m_actorSalt++;
				return newActor;
			}

		}

		//only get here if there were no empty spaces
		ActorUID nextUID = ActorUID(static_cast<int>(m_allActors.size()), m_actorSalt);
		Actor* newActor = new Actor(nextUID, definition, this, position, orientation, velocity);
		m_allActors.push_back(newActor);
		newActor->Startup();
		m_actorSalt++;
		return newActor;
	}

	return nullptr;
}


Actor* Map::SpawnProjectile(std::string projectileDefName, Vec3 const& position, EulerAngles const& orientation, Vec3 const& velocity, Actor* projectileOwner)
{
	ActorDefinition const* definition = ActorDefinition::GetProjectileActorDefinition(projectileDefName);

	if (definition != nullptr)
	{
		//loop through actor list to find empty slot
		for (int actorIndex = 0; actorIndex < m_allActors.size(); actorIndex++)
		{
			if (m_allActors[actorIndex] == nullptr)
			{
				ActorUID nextUID = ActorUID(actorIndex, m_actorSalt);
				Actor* newActor = new Actor(nextUID, definition, this, position, orientation, velocity);
				newActor->m_projectileOwner = projectileOwner;
				m_allActors.push_back(newActor);
				newActor->Startup();
				m_actorSalt++;
				return newActor;
			}

		}

		//only get here if there were no empty spaces
		ActorUID nextUID = ActorUID(static_cast<int>(m_allActors.size()), m_actorSalt);
		Actor* newActor = new Actor(nextUID, definition, this, position, orientation, velocity);
		newActor->m_projectileOwner = projectileOwner;
		m_allActors.push_back(newActor);
		newActor->Startup();
		m_actorSalt++;
		return newActor;
	}

	return nullptr;
}


//
//public collision functions
//
void Map::CollideAllActorsWithEachOther()
{
	for (int actorIndexA = 0; actorIndexA < m_allActors.size(); actorIndexA++)
	{
		for (int actorIndexB = actorIndexA + 1; actorIndexB < m_allActors.size(); actorIndexB++)
		{
			Actor*& actorA = m_allActors[actorIndexA];
			Actor*& actorB = m_allActors[actorIndexB];

			if (actorA != nullptr && actorA->m_definition->m_collideWithActors && actorA->m_health > 0 && actorB != nullptr && actorB->m_definition->m_collideWithActors && actorB->m_health > 0)
			{
				CollideActorsWithEachOther(actorA, actorB);
			}
		}
	}
}


void Map::CollideActorsWithEachOther(Actor* actorA, Actor* actorB)
{
	//return if not overlapping on z axis
	Vec3& posA = actorA->m_position;
	Vec3& posB = actorB->m_position;
	float radiusA = actorA->m_physicsRadius;
	float radiusB = actorB->m_physicsRadius;
	float heightA = actorA->m_physicsHeight;
	float heightB = actorB->m_physicsHeight;

	FloatRange actorARange = FloatRange(posA.z, posA.z + heightA);
	FloatRange actorBRange = FloatRange(posB.z, posB.z + heightB);
	if (!actorARange.IsOverlappingWith(actorBRange))
	{
		return;
	}

	if (actorA == actorB->m_projectileOwner || actorB == actorA->m_projectileOwner)
	{
		return;
	}
	
	if (actorA->m_isStatic && actorB->m_isStatic)
	{
		return;
	}
	else if (actorA->m_isStatic && !actorB->m_isStatic)
	{
		bool didCollide = PushDiscOutOfFixedDisc2D(posB, radiusB, posA, radiusA);
		if (didCollide)
		{
			actorA->OnCollideWithActor(actorB);
			actorB->OnCollideWithActor(actorA);
		}
	}
	else if (!actorA->m_isStatic && actorB->m_isStatic)
	{
		bool didCollide = PushDiscOutOfFixedDisc2D(posA, radiusA, posB, radiusB);
		if (didCollide)
		{
			actorA->OnCollideWithActor(actorB);
			actorB->OnCollideWithActor(actorA);
		}
	}
	else
	{
		bool didCollide = PushDiscsOutOfEachOther2D(posA, radiusA, posB, radiusB);
		if (didCollide)
		{
			actorA->OnCollideWithActor(actorB);
			actorB->OnCollideWithActor(actorA);
		}
	}
}


void Map::CollideAllActorsWithMap()
{
	for (int actorIndex = 0; actorIndex < m_allActors.size(); actorIndex++)
	{
		Actor*& actor = m_allActors[actorIndex];

		if (actor != nullptr && actor->m_definition->m_collideWithWorld && actor->m_health > 0)
		{
			CollideActorWithMap(actor);
		}
	}
}


void Map::CollideActorWithMap(Actor* actor)
{
	//push out of all 8 neighboring walls
	int tileID = GetTileIDFromPosition(actor->m_position);

	CollideActorWithTile(actor, &m_tiles[tileID + m_dimensions.x]);
	CollideActorWithTile(actor, &m_tiles[tileID + 1]);
	CollideActorWithTile(actor, &m_tiles[tileID - m_dimensions.x]);
	CollideActorWithTile(actor, &m_tiles[tileID - 1]);
	CollideActorWithTile(actor, &m_tiles[tileID + m_dimensions.x + 1]);
	CollideActorWithTile(actor, &m_tiles[tileID - m_dimensions.x + 1]);
	CollideActorWithTile(actor, &m_tiles[tileID - m_dimensions.x - 1]);
	CollideActorWithTile(actor, &m_tiles[tileID + m_dimensions.x - 1]);

	//push out of floor/ceiling
	if (actor->m_position.z < 0.0f)
	{
		actor->m_position.z = 0.0f;
		actor->OnCollide();
	}
	if (actor->m_position.z + actor->m_physicsHeight > 1.0f)
	{
		actor->m_position.z = 1.0f - actor->m_physicsHeight;
		actor->OnCollide();
	}
}


void Map::CollideActorWithTile(Actor* actor, Tile const* tile)
{
	if (tile != nullptr && tile->m_definition->m_isSolid)
	{
		bool didCollide = PushDiscOutOfFixedAABB2D(actor->m_position, actor->m_physicsRadius, tile->GetTileAABB2());
		if (didCollide)
		{
			actor->OnCollide();
		}
	}
}


//
//public raycast functions
//
RaycastResultGame Map::RaycastAgainstAll(Vec3 const& startPosition, Vec3 const& directionNormal, float distance, Actor* owner)
{
	RaycastResultGame raycastResultActors = RaycastAgainstActors(startPosition, directionNormal, distance, owner);
	RaycastResult3D raycastResultWorldXY = RaycastAgainstTilesXY(startPosition, directionNormal, distance);
	RaycastResult3D raycastResultWorldZ = RaycastAgainstTilesZ(startPosition, directionNormal, distance);

	RaycastResultGame raycastResult;
	raycastResult.m_raycastResult.m_impactDist = distance + 1.0f;
	if (raycastResultActors.m_raycastResult.m_didImpact && raycastResultActors.m_raycastResult.m_impactDist < raycastResult.m_raycastResult.m_impactDist && IsPositionInBounds(raycastResultActors.m_raycastResult.m_impactPos))
	{
		raycastResult = raycastResultActors;
	}
	if (raycastResultWorldXY.m_didImpact && raycastResultWorldXY.m_impactDist < raycastResult.m_raycastResult.m_impactDist && IsPositionInBounds(raycastResultWorldXY.m_impactPos))
	{
		raycastResult.m_raycastResult = raycastResultWorldXY;
		raycastResult.m_actorHit = nullptr;
	}
	if (raycastResultWorldZ.m_didImpact && raycastResultWorldZ.m_impactDist < raycastResult.m_raycastResult.m_impactDist && IsPositionInBounds(raycastResultWorldZ.m_impactPos))
	{
		raycastResult.m_raycastResult = raycastResultWorldZ;
		raycastResult.m_actorHit = nullptr;
	}
	
	/*DebugAddWorldLine(startPosition, startPosition + (directionNormal * distance), 0.01f, 10.0f, Rgba8(), Rgba8(), DebugRenderMode::X_RAY);

	if (raycastResult.m_raycastResult.m_didImpact)
	{
		DebugAddWorldSphere(raycastResult.m_raycastResult.m_impactPos, 0.06f, 10.0f);
		DebugAddWorldArrow(raycastResult.m_raycastResult.m_impactPos, raycastResult.m_raycastResult.m_impactPos + (raycastResult.m_raycastResult.m_impactNormal * 0.3f), 0.03f, 10.0f, Rgba8(0, 0, 255), Rgba8(0, 0, 255));
	}*/
	
	return raycastResult;
}


RaycastResultGame Map::RaycastAgainstActors(Vec3 const& startPosition, Vec3 const& directionNormal, float distance, Actor* owner)
{
	RaycastResultGame raycastResult;
	raycastResult.m_raycastResult.m_impactDist = distance + 1.0f;
	raycastResult.m_owner = owner;

	for (int actorIndex = 0; actorIndex < m_allActors.size(); actorIndex++)
	{
		Actor* actor = m_allActors[actorIndex];
		if (actor != nullptr && actor != raycastResult.m_owner && actor->m_health > 0)
		{
			Vec3 actorPos = actor->m_position;
			RaycastResult3D raycastResultActor = RaycastVsZCylinder3D(startPosition, directionNormal, distance, actorPos, actorPos.z, actorPos.z + actor->m_physicsHeight, actor->m_physicsRadius);

			//if raycast hit, has shorter distance than previously saved raycast, and is within bounds, save this one instead
			if (raycastResultActor.m_didImpact && raycastResultActor.m_impactDist < raycastResult.m_raycastResult.m_impactDist && IsPositionInBounds(raycastResultActor.m_impactPos))
			{
				raycastResult.m_raycastResult = raycastResultActor;
				raycastResult.m_actorHit = actor;
			}
		}
	}

	raycastResult.m_raycastResult.m_rayStartPosition = startPosition;
	raycastResult.m_raycastResult.m_rayDirection = directionNormal;
	raycastResult.m_raycastResult.m_rayLength = distance;

	return raycastResult;
}


RaycastResultGame Map::RaycastAgainstPlayers(Vec3 const& startPosition, Vec3 const& directionNormal, float distance, Actor* owner /*= nullptr*/)
{
	RaycastResultGame raycastResult;
	raycastResult.m_raycastResult.m_impactDist = distance + 1.0f;
	raycastResult.m_owner = owner;

	for (int playerIndex = 0; playerIndex < m_players.size(); playerIndex++)
	{
		if (m_players[playerIndex] != nullptr)
		{
			Actor* playerActor = m_players[playerIndex]->GetActor();
			if (playerActor != nullptr && playerActor != raycastResult.m_owner && playerActor->m_health > 0)
			{
				Vec3 actorPos = playerActor->m_position;
				RaycastResult3D raycastResultActor = RaycastVsZCylinder3D(startPosition, directionNormal, distance, actorPos, actorPos.z, actorPos.z + playerActor->m_physicsHeight, playerActor->m_physicsRadius);

				//if raycast hit, has shorter distance than previously saved raycast, and is within bounds, save this one instead
				if (raycastResultActor.m_didImpact && raycastResultActor.m_impactDist < raycastResult.m_raycastResult.m_impactDist && IsPositionInBounds(raycastResultActor.m_impactPos))
				{
					raycastResult.m_raycastResult = raycastResultActor;
					raycastResult.m_actorHit = playerActor;
				}
			}
		}
	}

	raycastResult.m_raycastResult.m_rayStartPosition = startPosition;
	raycastResult.m_raycastResult.m_rayDirection = directionNormal;
	raycastResult.m_raycastResult.m_rayLength = distance;

	return raycastResult;
}


RaycastResult3D Map::RaycastAgainstTilesXY(Vec3 const& startPosition, Vec3 const& directionNormal, float distance) const
{
	RaycastResult3D raycastResult;

	Vec3 raycastVector = directionNormal * distance;

	IntVec2 currentTileCoords = IntVec2(static_cast<int>(startPosition.x), static_cast<int>(startPosition.y));
	Tile const* tile = GetTileAtCoords(currentTileCoords.x, currentTileCoords.y);
	if (tile->m_definition->m_isSolid)
	{
		raycastResult.m_didImpact = true;
		raycastResult.m_impactDist = 0.0f;
		raycastResult.m_impactPos = startPosition;
		raycastResult.m_impactNormal = Vec3() - directionNormal;
	}
	else
	{
		float forwardDistPerX = 1.0f / fabsf(directionNormal.x);

		int tileStepDirectionX;
		if (directionNormal.x < 0)
		{
			tileStepDirectionX = -1;
		}
		else
		{
			tileStepDirectionX = 1;
		}

		float xAtFirstXCrossing = static_cast<float>(currentTileCoords.x + (tileStepDirectionX + 1)/2);
		float xDistToFirstXCrossing = xAtFirstXCrossing - startPosition.x;
		float totalDistAtNextXCrossing = fabsf(xDistToFirstXCrossing) * forwardDistPerX;

		float forwardDistPerY = 1.0f / fabsf(directionNormal.y);

		int tileStepDirectionY;
		if (directionNormal.y < 0)
		{
			tileStepDirectionY = -1;
		}
		else
		{
			tileStepDirectionY = 1;
		}

		float yAtFirstYCrossing = static_cast<float>(currentTileCoords.y + (tileStepDirectionY + 1)/2);
		float yDistToFirstYCrossing = yAtFirstYCrossing - startPosition.y;
		float totalDistAtNextYCrossing = fabsf(yDistToFirstYCrossing) * forwardDistPerY;

		while (totalDistAtNextXCrossing < distance || totalDistAtNextYCrossing < distance)
		{
			if (totalDistAtNextXCrossing < totalDistAtNextYCrossing)
			{
				if (totalDistAtNextXCrossing > distance)
				{
					raycastResult.m_rayStartPosition = startPosition;
					raycastResult.m_rayDirection = directionNormal;
					raycastResult.m_rayLength = distance;

					return raycastResult;
				}

				currentTileCoords.x += tileStepDirectionX;
				tile = GetTileAtCoords(currentTileCoords.x, currentTileCoords.y);
				if (tile->m_definition->m_isSolid)
				{
					raycastResult.m_didImpact = true;
					raycastResult.m_impactDist = totalDistAtNextXCrossing;
					raycastResult.m_impactPos = startPosition + (directionNormal * totalDistAtNextXCrossing);
					if (tileStepDirectionX == -1)
					{
						raycastResult.m_impactNormal = Vec3(1.0f, 0.0f, 0.0f);
					}
					else
					{
						raycastResult.m_impactNormal = Vec3(-1.0f, 0.0f, 0.0f);
					}
					raycastResult.m_rayStartPosition = startPosition;
					raycastResult.m_rayDirection = directionNormal;
					raycastResult.m_rayLength = distance;

					return raycastResult;
				}

				totalDistAtNextXCrossing += forwardDistPerX;
			}
			else
			{
				if (totalDistAtNextYCrossing > distance)
				{
					raycastResult.m_rayStartPosition = startPosition;
					raycastResult.m_rayDirection = directionNormal;
					raycastResult.m_rayLength = distance;

					return raycastResult;
				}

				currentTileCoords.y += tileStepDirectionY;
				tile = GetTileAtCoords(currentTileCoords.x, currentTileCoords.y);
				if (tile->m_definition->m_isSolid)
				{
					raycastResult.m_didImpact = true;
					raycastResult.m_impactDist = totalDistAtNextYCrossing;
					raycastResult.m_impactPos = startPosition + (directionNormal * totalDistAtNextYCrossing);
					if (tileStepDirectionY == -1)
					{
						raycastResult.m_impactNormal = Vec3(0.0f, 1.0f, 0.0f);
					}
					else
					{
						raycastResult.m_impactNormal = Vec3(0.0f, -1.0f, 0.0f);
					}
					raycastResult.m_rayStartPosition = startPosition;
					raycastResult.m_rayDirection = directionNormal;
					raycastResult.m_rayLength = distance;

					return raycastResult;
				}

				totalDistAtNextYCrossing += forwardDistPerY;
			}
		}
	}

	raycastResult.m_rayStartPosition = startPosition;
	raycastResult.m_rayDirection = directionNormal;
	raycastResult.m_rayLength = distance;

	return raycastResult;
}


RaycastResult3D Map::RaycastAgainstTilesZ(Vec3 const& startPosition, Vec3 const& directionNormal, float distance) const
{
	RaycastResult3D raycastResult;

	Vec3 raycastVector = directionNormal * distance;

	if (raycastVector.z > 0.0f)
	{
		float t = (1.0f - startPosition.z) / raycastVector.z;
		Vec3 posAtT = startPosition + (raycastVector * t);

		if (t > 0.0f && t < 1.0f)
		{
			raycastResult.m_didImpact = true;
			raycastResult.m_impactDist = GetDistance3D(posAtT, startPosition);
			raycastResult.m_impactPos = posAtT;
			raycastResult.m_impactNormal = Vec3(0.00001f, 0.0f, -1.0f);	//for some reason, the arrow doesn't render when facing purely -z and I don't know why
		}
	}
	else if (raycastVector.z < 0.0f)
	{
		float t = (-startPosition.z) / raycastVector.z;
		Vec3 posAtT = startPosition + (raycastVector * t);

		if (t > 0.0f && t < 1.0f)
		{
			raycastResult.m_didImpact = true;
			raycastResult.m_impactDist = GetDistance3D(posAtT, startPosition);
			raycastResult.m_impactPos = posAtT;
			raycastResult.m_impactNormal = Vec3(0.0f, 0.0f, 1.0f);
		}
	}

	raycastResult.m_rayStartPosition = startPosition;
	raycastResult.m_rayDirection = directionNormal;
	raycastResult.m_rayLength = distance;

	return raycastResult;
}


//
//public accessors
//
Tile const* Map::GetTileAtCoords(int x, int y) const
{
	int tileID = x + (y * m_dimensions.x);
	return &m_tiles[tileID];
}


Tile const* Map::GetTileAtPosition(Vec3 const& position) const
{
	int tileID = static_cast<int>(position.x) + (static_cast<int>(position.y) * m_dimensions.x);
	return &m_tiles[tileID];
}


int Map::GetTileIDFromCoords(int x, int y) const
{
	return x + (y * m_dimensions.x);
}


int Map::GetTileIDFromPosition(Vec3 const& position) const
{
	return static_cast<int>(position.x) + (static_cast<int>(position.y) * m_dimensions.x);
}


bool Map::IsPositionInBounds(Vec3 const& position, float tolerance) const
{
	if (position.x < -tolerance || position.y < -tolerance || position.x > static_cast<float>(m_dimensions.x) + tolerance || position.y > static_cast<float>(m_dimensions.y) + tolerance)
	{
		return false;
	}
	else if (position.z < -tolerance || position.z > 1.0f + tolerance)
	{
		return false;
	}

	return true;
}


bool Map::AreCoordsInBounds(int x, int y) const
{
	if (x < 0 || y < 0 || x > m_dimensions.x || y > m_dimensions.y)
	{
		return false;
	}

	return true;
}


Actor* Map::GetActorByUID(ActorUID uid) const
{
	if (uid.m_data == ActorUID::INVALID)
	{
		return nullptr;
	}

	unsigned int actorIndex = uid.GetIndex();

	if (m_allActors[actorIndex] == nullptr)
	{
		return nullptr;
	}

	else if (m_allActors[actorIndex]->m_UID.m_data != uid.m_data)
	{
		return nullptr;
	}

	return m_allActors[actorIndex];
}


Actor* Map::GetClosestVisibleEnemy(ActorFaction enemyFaction, Actor* requestor) const
{
	float closestEnemyDistance = FLT_MAX;
	Actor* closestEnemy = nullptr;

	for (int actorIndex = 0; actorIndex < m_allActors.size(); actorIndex++)
	{
		Actor* target = m_allActors[actorIndex];
		if (target == nullptr)
		{
			continue;
		}

		if (target->m_definition->m_faction != enemyFaction)
		{
			continue;
		}

		float targetDistance = GetDistance3D(requestor->m_position, target->m_position);
		if (targetDistance > requestor->m_definition->m_sightRadius * 0.5f)
		{
			continue;
		}

		Vec3 targetDisplacement = target->m_position - requestor->m_position;
		Vec2 targetDisplacementXY = Vec2(targetDisplacement.x, targetDisplacement.y);
		float targetDisplacementAngle = GetAngleDegreesBetweenVectors2D(requestor->GetModelMatrixYawOnly().GetIBasis2D(), targetDisplacementXY);
		if (targetDisplacementAngle > requestor->m_definition->m_sightAngle)
		{
			continue;
		}

		RaycastResult3D sightLineCast = RaycastAgainstTilesXY(requestor->m_position, targetDisplacement.GetNormalized(), requestor->m_definition->m_sightRadius);
		if (sightLineCast.m_didImpact && sightLineCast.m_impactDist < targetDistance)
		{
			continue;
		}

		if (targetDistance < closestEnemyDistance)
		{
			closestEnemyDistance = targetDistance;
			closestEnemy = target;
		}
	}

	return closestEnemy;
}


//
//debug function for possessing next actor
//
void Map::DebugPossessNext()
{
	unsigned int nextActorIndex = m_players[0]->m_actorUID.GetIndex() + 1;

	while (true)
	{
		if (nextActorIndex >= m_allActors.size())
		{
			nextActorIndex = 0;
		}
		if (m_allActors[nextActorIndex] == nullptr)
		{
			nextActorIndex++;
			continue;
		}
		else if (!m_allActors[nextActorIndex]->m_definition->m_canBePossessed)
		{
			nextActorIndex++;
			continue;
		}
		else
		{
			m_players[0]->Possess(m_allActors[nextActorIndex]->m_UID);
			if (m_currentPlayerActors[0]->m_AIController != nullptr)
			{
				m_currentPlayerActors[0]->m_AIController->Possess(m_currentPlayerActors[0]->m_UID);
			}
			m_currentPlayerActors[0] = m_allActors[nextActorIndex];
			break;
		}
	}
}


//
//light constants function for lights out mode
//
void Map::SetFlashlightConstants(Vec3 flashlightPosition, float flashlightIntensity, float flashlightSize, Vec3 flashlightAtt)
{
	FlashlightConstants flashlightConstants;
	flashlightConstants.FlashlightPosition = flashlightPosition;
	flashlightConstants.FlashlightIntensity = flashlightIntensity;
	flashlightConstants.FlashlightSize = flashlightSize;
	flashlightConstants.FlashlightAtt = flashlightAtt;

	g_theRenderer->CopyCPUToGPU(&flashlightConstants, sizeof(flashlightConstants), m_flashlightConstants);
	g_theRenderer->BindConstantBuffer(k_lightConstantsSlot, m_flashlightConstants);
}
