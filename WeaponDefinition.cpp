#include "Game/WeaponDefinition.hpp"
#include "Game/Game.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"


//static variable declaration
std::vector<WeaponDefinition> WeaponDefinition::s_weaponDefinitions;


//
//constructor
//
WeaponDefinition::WeaponDefinition(XmlElement const& element)
{
	m_name = ParseXmlAttribute(element, "name", m_name);
	m_refireTime = ParseXmlAttribute(element, "refireTime", m_refireTime);
	m_rayCount = ParseXmlAttribute(element, "rayCount", m_rayCount);
	m_rayCone = ParseXmlAttribute(element, "rayCone", m_rayCone);
	m_rayRange = ParseXmlAttribute(element, "rayRange", m_rayRange);
	m_rayDamage = ParseXmlAttribute(element, "rayDamage", m_rayDamage);
	m_rayImpulse = ParseXmlAttribute(element, "rayImpulse", m_rayImpulse);
	m_projectileCount = ParseXmlAttribute(element, "projectileCount", m_projectileCount);
	m_projectileCone = ParseXmlAttribute(element, "projectileCone", m_projectileCone);
	m_projectileSpeed = ParseXmlAttribute(element, "projectileSpeed", m_projectileSpeed);
	m_projectileActor = ParseXmlAttribute(element, "projectileActor", m_projectileActor);
	m_meleeCount = ParseXmlAttribute(element, "meleeCount", m_meleeCount);
	m_meleeRange = ParseXmlAttribute(element, "meleeRange", m_meleeRange);
	m_meleeArc = ParseXmlAttribute(element, "meleeArc", m_meleeArc);
	m_meleeDamage = ParseXmlAttribute(element, "meleeDamage", m_meleeDamage);
	m_meleeImpulse = ParseXmlAttribute(element, "meleeImpulse", m_meleeImpulse);
	m_holdToUse = ParseXmlAttribute(element, "holdToUse", m_holdToUse);

	m_lightIntensity = ParseXmlAttribute(element, "lightIntensity", m_lightIntensity);
	m_focusLightIntensity = ParseXmlAttribute(element, "focusLightIntensity", m_focusLightIntensity);
	m_lightRadius = ParseXmlAttribute(element, "lightRadius", m_lightRadius);
	m_focusLightRadius = ParseXmlAttribute(element, "focusLightRadius", m_focusLightRadius);
	m_focusLightDamage = ParseXmlAttribute(element, "focusLightDamage", m_focusLightDamage);
	m_focusLightDamageInterval = ParseXmlAttribute(element, "focusLightDamageInterval", m_focusLightDamageInterval);
	m_focusLightImpulse = ParseXmlAttribute(element, "focusLightImpulse", m_focusLightImpulse);
	m_focusLightRange = ParseXmlAttribute(element, "focusLightRange", m_focusLightRange);

	XmlElement const* parameterElement = element.FirstChildElement();
	std::string elementName;
	if (parameterElement != nullptr)
	{
		elementName = parameterElement->Name();
	}

	//parse hud parameters
	if (parameterElement != nullptr && elementName == "HUD")
	{
		std::string shaderFilePath = ParseXmlAttribute(*parameterElement, "shader", "Data/Shaders/Default");
		m_hudShader = g_theRenderer->CreateShader(shaderFilePath.c_str());

		std::string hudTextureFilePath = ParseXmlAttribute(*parameterElement, "baseTexture", "invalid file path");
		m_baseTexture = g_theRenderer->CreateOrGetTextureFromFile(hudTextureFilePath.c_str());

		std::string reticleTextureFilePath = ParseXmlAttribute(*parameterElement, "reticleTexture", "invalid file path");
		m_reticleTexture = g_theRenderer->CreateOrGetTextureFromFile(reticleTextureFilePath.c_str());

		m_reticleSize = ParseXmlAttribute(*parameterElement, "reticleSize", m_reticleSize);
		m_spriteSize = ParseXmlAttribute(*parameterElement, "spriteSize", m_spriteSize);
		m_spritePivot = ParseXmlAttribute(*parameterElement, "spritePivot", m_spritePivot);

		//parse animation parameters
		XmlElement const* animElement = parameterElement->FirstChildElement();
		while (animElement != nullptr)
		{
			std::string animElementName = animElement->Name();

			if (animElementName == "Animation")
			{
				std::string animName = ParseXmlAttribute(*animElement, "name", "invalid name");
				
				std::string weaponShaderFilePath = ParseXmlAttribute(*animElement, "shader", "Data/Shaders/Default");
				Shader* shader = g_theRenderer->CreateShader(weaponShaderFilePath.c_str());

				std::string spriteSheetFilePath = ParseXmlAttribute(*animElement, "spriteSheet", "invalid texture");
				Texture* spriteSheetTexture = g_theRenderer->CreateOrGetTextureFromFile(spriteSheetFilePath.c_str());
				IntVec2 spriteSheetCellCount = ParseXmlAttribute(*animElement, "cellCount", IntVec2(1, 1));
				SpriteSheet* spriteSheet = new SpriteSheet(*spriteSheetTexture, spriteSheetCellCount);

				float secondsPerFrame = ParseXmlAttribute(*animElement, "secondsPerFrame", 1.0f);
				int startFrame = ParseXmlAttribute(*animElement, "startFrame", 0);
				int endFrame = ParseXmlAttribute(*animElement, "endFrame", 0);

				m_weaponAnimDefs.push_back(WeaponAnimDef(animName, shader, spriteSheet, startFrame, endFrame, secondsPerFrame, SpriteAnimPlaybackType::ONCE));
			}

			animElement = animElement->NextSiblingElement();
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
	}
}


//
//static functions
//
void WeaponDefinition::InitializeWeaponDefs()
{
	XmlDocument weaponDefsXml;
	char const* filePath = "Data/Definitions/WeaponDefinitions.xml";
	XmlError result = weaponDefsXml.LoadFile(filePath);
	GUARANTEE_OR_DIE(result == tinyxml2::XML_SUCCESS, "Failed to open weapon definitions xml file!");

	XmlElement* rootElement = weaponDefsXml.RootElement();
	GUARANTEE_OR_DIE(rootElement != nullptr, "Failed to read weapon definitions root element!");

	XmlElement* weaponDefElement = rootElement->FirstChildElement();
	while (weaponDefElement != nullptr)
	{
		std::string elementName = weaponDefElement->Name();
		GUARANTEE_OR_DIE(elementName == "WeaponDefinition", "Child element names in weapon definitions xml file must be <WeaponDefinition>!");
		WeaponDefinition newWeaponDef = WeaponDefinition(*weaponDefElement);
		s_weaponDefinitions.push_back(newWeaponDef);
		weaponDefElement = weaponDefElement->NextSiblingElement();
	}
}


WeaponDefinition const* WeaponDefinition::GetWeaponDefinition(std::string name)
{
	for (int defIndex = 0; defIndex < s_weaponDefinitions.size(); defIndex++)
	{
		if (s_weaponDefinitions[defIndex].m_name == name)
		{
			return &s_weaponDefinitions[defIndex];
		}
	}

	//return null if it wasn't found
	return nullptr;
}
