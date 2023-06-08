#include "Game/ActorDefinition.hpp"
#include "Game/Game.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/Renderer.hpp"


//static variable declaration
std::vector<ActorDefinition> ActorDefinition::s_actorDefinitions;
std::vector<ActorDefinition> ActorDefinition::s_projectileActorDefinitions;


//
//constructor
//
ActorDefinition::ActorDefinition(XmlElement const& element)
{
	//parse base parameters
	m_name = ParseXmlAttribute(element, "name", m_name);
	m_isVisible = ParseXmlAttribute(element, "visible", m_isVisible);
	m_maxHealth = ParseXmlAttribute(element, "health", m_maxHealth);
	m_corpseLifetime = ParseXmlAttribute(element, "corpseLifetime", m_corpseLifetime);
	std::string factionString = ParseXmlAttribute(element, "faction", "Neutral");
	if (factionString == "Marine")
	{
		m_faction = ActorFaction::MARINE;
	}
	else if (factionString == "Demon")
	{
		m_faction = ActorFaction::DEMON;
	}
	m_canBePossessed = ParseXmlAttribute(element, "canBePossessed", m_canBePossessed);
	m_dieOnSpawn = ParseXmlAttribute(element, "dieOnSpawn", m_dieOnSpawn);
	m_immuneToLight = ParseXmlAttribute(element, "immuneToLight", m_immuneToLight);
	
	XmlElement const* parameterElement = element.FirstChildElement();
	std::string elementName;
	if (parameterElement != nullptr)
	{
		elementName = parameterElement->Name();
	}

	//parse collision parameters
	if (parameterElement != nullptr && elementName == "Collision")
	{
		m_physicsHeight = ParseXmlAttribute(*parameterElement, "height", m_physicsHeight);
		m_physicsRadius = ParseXmlAttribute(*parameterElement, "radius", m_physicsRadius);
		m_collideWithWorld = ParseXmlAttribute(*parameterElement, "collidesWithWorld", m_collideWithWorld);
		m_collideWithActors = ParseXmlAttribute(*parameterElement, "collidesWithActors", m_collideWithActors);
		m_isPushable = ParseXmlAttribute(*parameterElement, "isPushable", m_isPushable);
		m_dieOnCollide = ParseXmlAttribute(*parameterElement, "dieOnCollide", m_dieOnCollide);
		m_damageOnCollide = ParseXmlAttribute(*parameterElement, "damageOnCollide", m_damageOnCollide);
		m_impulseOnCollide = ParseXmlAttribute(*parameterElement, "impulseOnCollide", m_impulseOnCollide);

		parameterElement = parameterElement->NextSiblingElement();

		if (parameterElement != nullptr)
		{
			elementName = parameterElement->Name();
		}
	}

	//parse physics parameters
	if (parameterElement != nullptr && elementName == "Physics")
	{
		m_isSimulated = ParseXmlAttribute(*parameterElement, "simulated", m_isSimulated);
		m_isFlying = ParseXmlAttribute(*parameterElement, "flying", m_isFlying);
		m_walkSpeed = ParseXmlAttribute(*parameterElement, "walkSpeed", m_walkSpeed);
		m_runSpeed = ParseXmlAttribute(*parameterElement, "runSpeed", m_runSpeed);
		m_drag = ParseXmlAttribute(*parameterElement, "drag", m_drag);
		m_turnSpeed = ParseXmlAttribute(*parameterElement, "turnSpeed", m_turnSpeed);

		parameterElement = parameterElement->NextSiblingElement();

		if (parameterElement != nullptr)
		{
			elementName = parameterElement->Name();
		}
	}

	//parse camera parameters
	if (parameterElement != nullptr && elementName == "Camera")
	{
		m_eyeHeight = ParseXmlAttribute(*parameterElement, "eyeHeight", m_eyeHeight);
		m_cameraFOVDegrees = ParseXmlAttribute(*parameterElement, "cameraFOV", m_cameraFOVDegrees);

		parameterElement = parameterElement->NextSiblingElement();

		if (parameterElement != nullptr)
		{
			elementName = parameterElement->Name();
		}
	}

	//parse ai parameters
	if (parameterElement != nullptr && elementName == "AI")
	{
		m_isAIEnabled = ParseXmlAttribute(*parameterElement, "aiEnabled", m_isAIEnabled);
		m_sightRadius = ParseXmlAttribute(*parameterElement, "sightRadius", m_sightRadius);
		m_sightAngle = ParseXmlAttribute(*parameterElement, "sightAngle", m_sightAngle);
		m_freezeWhenSeen = ParseXmlAttribute(*parameterElement, "freezeWhenSeen", m_freezeWhenSeen);

		parameterElement = parameterElement->NextSiblingElement();

		if (parameterElement != nullptr)
		{
			elementName = parameterElement->Name();
		}
	}

	//parse visual parameters
	if (parameterElement != nullptr && elementName == "Visuals")
	{
		m_spriteSize = ParseXmlAttribute(*parameterElement, "size", m_spriteSize);
		m_spritePivot = ParseXmlAttribute(*parameterElement, "pivot", m_spritePivot);
		std::string billboardString = ParseXmlAttribute(*parameterElement, "billboardType", "None");
		if (billboardString == "WorldUpFacing")
		{
			m_billboardType = BillboardType::WORLD_UP_CAMERA_FACING;
		}
		else if (billboardString == "WorldUpOpposing")
		{
			m_billboardType = BillboardType::WORLD_UP_CAMERA_OPPOSING;
		}
		else if (billboardString == "FullOpposing")
		{
			m_billboardType = BillboardType::FULL_CAMERA_OPPOSING;
		}
		m_renderLit = ParseXmlAttribute(*parameterElement, "renderLit", m_renderLit);
		m_renderRounded = ParseXmlAttribute(*parameterElement, "renderRounded", m_renderRounded);

		std::string shaderFilePath = ParseXmlAttribute(*parameterElement, "shader", "Data/Shaders/Default");
		m_shader = g_theRenderer->CreateShader(shaderFilePath.c_str());

		std::string spriteSheetFilePath = ParseXmlAttribute(*parameterElement, "spriteSheet", "invalid sprite sheet file path");
		m_spriteSheetTexture = g_theRenderer->CreateOrGetTextureFromFile(spriteSheetFilePath.c_str());
		m_spriteSheetCellCount = ParseXmlAttribute(*parameterElement, "cellCount", m_spriteSheetCellCount);

		m_spriteSheet = new SpriteSheet(*m_spriteSheetTexture, m_spriteSheetCellCount);
		
		//parse anim group defs
		XmlElement const* groupElement = parameterElement->FirstChildElement();
		while (groupElement != nullptr)
		{
			std::string groupElementName = groupElement->Name();
			
			if (groupElementName == "AnimationGroup")
			{
				SpriteAnimGroupDef animGroupDef = SpriteAnimGroupDef(this);
				animGroupDef.LoadFromXMLElement(*groupElement);
				m_animGroupDefs.push_back(animGroupDef);
			}
			
			groupElement = groupElement->NextSiblingElement();
		}

		parameterElement = parameterElement->NextSiblingElement();

		if (parameterElement != nullptr)
		{
			elementName = parameterElement->Name();
		}
	}

	//parse sound parameters
	if (parameterElement != nullptr && elementName == "Sounds")
	{
		XmlElement const* soundElement = parameterElement->FirstChildElement();
		while (soundElement != nullptr)
		{
			std::string soundElementName = soundElement->Name();
			if (soundElementName == "Sound")
			{
				std::string soundName = ParseXmlAttribute(*soundElement, "sound", "invalid sound name");
				m_soundNames.push_back(soundName);

				std::string soundFilePath = ParseXmlAttribute(*soundElement, "name", "invalid file path");
				m_sounds.push_back(g_theAudio->CreateOrGetSound(soundFilePath, true));
			}

			soundElement = soundElement->NextSiblingElement();
		}

		parameterElement = parameterElement->NextSiblingElement();

		if (parameterElement != nullptr)
		{
			elementName = parameterElement->Name();
		}
	}

	//parse weapon parameters
	if (parameterElement != nullptr && elementName == "Inventory")
	{
		XmlElement const* inventoryElement = parameterElement->FirstChildElement();
		while (inventoryElement != nullptr)
		{
			std::string inventoryElementName = inventoryElement->Name();

			if (inventoryElementName == "Weapon")
			{
				std::string weaponName = ParseXmlAttribute(*inventoryElement, "name", "invalid weapon element");
				m_weapons.push_back(weaponName);
			}

			inventoryElement = inventoryElement->NextSiblingElement();
		}
	}
}


//
//static functions
//
void ActorDefinition::InitializeActorDefs()
{
	XmlDocument actorDefsXml;
	char const* filePath = "Data/Definitions/ActorDefinitions.xml";
	XmlError result = actorDefsXml.LoadFile(filePath);
	GUARANTEE_OR_DIE(result == tinyxml2::XML_SUCCESS, "Failed to open actor definitions xml file!");

	XmlElement* rootElement = actorDefsXml.RootElement();
	GUARANTEE_OR_DIE(rootElement != nullptr, "Failed to read actor definitions root element!");

	XmlElement* actorDefElement = rootElement->FirstChildElement();
	while (actorDefElement != nullptr)
	{
		std::string elementName = actorDefElement->Name();
		GUARANTEE_OR_DIE(elementName == "ActorDefinition", "Child element names in actor definitions xml file must be <ActorDefinition>!");
		ActorDefinition newActorDef = ActorDefinition(*actorDefElement);
		s_actorDefinitions.push_back(newActorDef);
		actorDefElement = actorDefElement->NextSiblingElement();
	}
}


void ActorDefinition::InitializeProjectileActorDefs()
{
	XmlDocument projectileActorDefsXml;
	char const* filePath = "Data/Definitions/ProjectileActorDefinitions.xml";
	XmlError result = projectileActorDefsXml.LoadFile(filePath);
	GUARANTEE_OR_DIE(result == tinyxml2::XML_SUCCESS, "Failed to open projectile actor definitions xml file!");

	XmlElement* rootElement = projectileActorDefsXml.RootElement();
	GUARANTEE_OR_DIE(rootElement != nullptr, "Failed to read projectile actor definitions root element!");

	XmlElement* projectileActorDefElement = rootElement->FirstChildElement();
	while (projectileActorDefElement != nullptr)
	{
		std::string elementName = projectileActorDefElement->Name();
		GUARANTEE_OR_DIE(elementName == "ActorDefinition", "Child element names in projectile actor definitions xml file must be <ActorDefinition>!");
		ActorDefinition newProjectileActorDef = ActorDefinition(*projectileActorDefElement);
		s_projectileActorDefinitions.push_back(newProjectileActorDef);
		projectileActorDefElement = projectileActorDefElement->NextSiblingElement();
	}
}


ActorDefinition const* ActorDefinition::GetActorDefinition(std::string name)
{
	for (int defIndex = 0; defIndex < s_actorDefinitions.size(); defIndex++)
	{
		if (s_actorDefinitions[defIndex].m_name == name)
		{
			return &s_actorDefinitions[defIndex];
		}
	}

	//return null if it wasn't found
	return nullptr;
}


ActorDefinition const* ActorDefinition::GetProjectileActorDefinition(std::string name)
{
	for (int defIndex = 0; defIndex < s_projectileActorDefinitions.size(); defIndex++)
	{
		if (s_projectileActorDefinitions[defIndex].m_name == name)
		{
			return &s_projectileActorDefinitions[defIndex];
		}
	}

	//return null if it wasn't found
	return nullptr;
}
