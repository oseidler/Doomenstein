#pragma once
#include "Game/Tile.hpp"
#include "Game/ActorUID.hpp"
#include "Game/ActorDefinition.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Vertex_PNCU.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Audio/AudioSystem.hpp"


//forward declarations
class MapDefinition;
class Game;
class Player;
class SpriteSheet;
class Actor;
class VertexBuffer;
class IndexBuffer;
class ConstantBuffer;


struct RaycastResultGame
{
	RaycastResult3D m_raycastResult;
	Actor* m_owner = nullptr;
	Actor* m_actorHit = nullptr;
};


class Map
{
//public member functions
public:
	//constructor
	Map(Game* owner, MapDefinition const* definition, int numPlayers, int player1XboxID, int player2XboxID);

	//game flow functions
	void Startup();
	void Update(float deltaSeconds);
	void Render(int currentPlayerRendering);
	void Shutdown();

	//actor handling functions
	void DeleteDestroyedActors();
	Actor* SpawnPlayer(int playerIndex);
	Actor* SpawnActor(std::string actorDefName, Vec3 const& position, EulerAngles const& orientation, Vec3 const& velocity = Vec3());
	Actor* SpawnProjectile(std::string projectileDefName, Vec3 const& position, EulerAngles const& orientation, Vec3 const& velocity = Vec3(), Actor* projectileOwner = nullptr);

	//collision functions
	void CollideAllActorsWithEachOther();
	void CollideActorsWithEachOther(Actor* actorA, Actor* actorB);
	void CollideAllActorsWithMap();
	void CollideActorWithMap(Actor* actor);
	void CollideActorWithTile(Actor* actor, Tile const* tile);

	//raycast functions
	RaycastResultGame RaycastAgainstAll(Vec3 const& startPosition, Vec3 const& directionNormal, float distance, Actor* owner = nullptr);
	RaycastResultGame RaycastAgainstActors(Vec3 const& startPosition, Vec3 const& directionNormal, float distance, Actor* owner = nullptr);
	RaycastResultGame RaycastAgainstPlayers(Vec3 const& startPosition, Vec3 const& directionNormal, float distance, Actor* owner = nullptr);
	RaycastResult3D RaycastAgainstTilesXY(Vec3 const& startPosition, Vec3 const& directionNormal, float distance) const;
	RaycastResult3D RaycastAgainstTilesZ(Vec3 const& startPosition, Vec3 const& directionNormal, float distance) const;

	//accessors
	Tile const* GetTileAtCoords(int x, int y) const;
	Tile const* GetTileAtPosition(Vec3 const& position) const;
	int			GetTileIDFromCoords(int x, int y) const;
	int			GetTileIDFromPosition(Vec3 const& position) const;
	bool		IsPositionInBounds(Vec3 const& position, float tolerance = 0.0f) const;
	bool		AreCoordsInBounds(int x, int y) const;
	Actor*		GetActorByUID(ActorUID uid) const;
	Actor*		GetClosestVisibleEnemy(ActorFaction enemyFaction, Actor* requestor) const;

	//debug function for possessing actors
	void DebugPossessNext();

	//light constants function for lights out mode
	void SetFlashlightConstants(Vec3 flashlightPosition, float flashlightIntensity, float flashlightSize, Vec3 flashlightAtt);

//public member variables
public:
	Game* m_owner;

	std::vector<Actor*> m_allActors;
	unsigned int		m_actorSalt;

	int m_numPlayers;
	std::vector<Player*> m_players;
	std::vector<Actor*>  m_currentPlayerActors;

	std::vector<Tile>	 m_tiles;
	MapDefinition const* m_definition;
	IntVec2				 m_dimensions;
	
	std::vector<Vertex_PNCU>  m_tileVerts;
	std::vector<unsigned int> m_tileVertIndexes;
	VertexBuffer*			  m_tileVertBuffer;
	IndexBuffer*			  m_tileIndexBuffer;
	ConstantBuffer*			  m_flashlightConstants;

	SpriteSheet* m_tileSpriteSheet;

	Vec3  m_sunDirection = Vec3(2.0f, 1.0f, -1.0f);
	float m_sunIntensity = 0.85f;
	float m_ambientIntensity = 0.35f;

	float m_lightsOutSunIntensity = 0.15f;
	float m_lightsOutAmbientIntensity = 0.08f;
};
