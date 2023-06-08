#include "Game/ActorUID.hpp"


//static variable initialization
unsigned int ActorUID::INVALID = 0xffffffff;


//
//constructor
//
ActorUID::ActorUID(unsigned int index, unsigned int salt)
{
	unsigned int maskedSalt = salt & 65535;
	unsigned int maskedIndex = index & 65535;

	unsigned int shiftedSalt = maskedSalt << 16;

	m_data = shiftedSalt | maskedIndex;
}


//
//public accessors
//
bool ActorUID::IsValid() const
{
	return m_data != INVALID;
}


unsigned int ActorUID::GetIndex() const
{
	return m_data & 65535;
}
