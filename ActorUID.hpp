#pragma once


class ActorUID
{
//public member functions
public:
	//constructor
	ActorUID(unsigned int index, unsigned int salt);

	//accessors
	bool IsValid() const;
	unsigned int GetIndex() const;

//public member variables
public:
	unsigned int m_data = INVALID;

	static unsigned int INVALID;
};
