#include "Game/SpriteAnimGroupDef.hpp"
#include "Game/ActorDefinition.hpp"


//
//constructor
//
SpriteAnimGroupDef::SpriteAnimGroupDef(ActorDefinition* actorDefinition)
	: m_actorDefinition(actorDefinition)
{
}


//
//parsing function
//
void SpriteAnimGroupDef::LoadFromXMLElement(XmlElement const& element)
{
	//parse base properties
	m_name = ParseXmlAttribute(element, "name", "invalid name");
	m_scaleBySpeed = ParseXmlAttribute(element, "scaleBySpeed", m_scaleBySpeed);
	m_secondsPerFrame = ParseXmlAttribute(element, "secondsPerFrame", m_secondsPerFrame);
	std::string playbackModeString = ParseXmlAttribute(element, "playbackMode", "Once");
	if (playbackModeString == "Once")
	{
		m_playbackMode = SpriteAnimPlaybackType::ONCE;
	}
	else if (playbackModeString == "Loop")
	{
		m_playbackMode = SpriteAnimPlaybackType::LOOP;
	}
	//ping-pong not implemented because I don't know what specific syntax the string would use

	//create directions and sprite anim defs
	XmlElement const* directionElement = element.FirstChildElement();
	while (directionElement != nullptr)
	{
		std::string directionElementName = directionElement->Name();

		if (directionElementName == "Direction")
		{
			Vec3 direction = ParseXmlAttribute(*directionElement, "vector", Vec3(1.0f, 0.0f, 0.0f));
			direction.Normalize();

			XmlElement const* animDefElement = directionElement->FirstChildElement();
			if (animDefElement != nullptr)
			{
				std::string animDefElementName = animDefElement->Name();
				if (animDefElementName == "Animation")
				{
					int startIndex = ParseXmlAttribute(*animDefElement, "startFrame", 0);
					int endIndex = ParseXmlAttribute(*animDefElement, "endFrame", 0);

					m_numFrames = endIndex - startIndex + 1;

					SpriteAnimDefinition spriteAnimDef = SpriteAnimDefinition(*m_actorDefinition->m_spriteSheet, startIndex, endIndex, 1.0f/m_secondsPerFrame, m_playbackMode);
					m_spriteAnimDefs.push_back(spriteAnimDef);
					m_directions.push_back(direction);
				}
			}
		}
		
		directionElement = directionElement->NextSiblingElement();
	}
}
