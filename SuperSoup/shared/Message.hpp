#pragma once

#include "Pair.hpp"

typedef unsigned int uint32; //todo add real typedef?
typedef unsigned short ushort16; //todo add real typedef?
typedef char* byte8; //todo add real typedef?

struct Message
{
	uint32 recpientID;
	Pair<ushort16, byte8*> datapair;
};