#pragma once
#include "Engine/Core/EngineCommon.hpp"


struct SpawnInfo
{
	std::string m_actorName = "invalid actor";
	Vec3		m_position = Vec3();
	EulerAngles m_orientation = EulerAngles();
	Vec3		m_velocity = Vec3();
};
