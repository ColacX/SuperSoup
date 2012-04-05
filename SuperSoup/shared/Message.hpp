#pragma once

struct Message
{
	typedef unsigned int uint32; //todo add real typedef?
	typedef unsigned short ushort16; //todo add real typedef?
	typedef char byte8; //todo add real typedef?
	
	uint32 recpientID;
	ushort16 messageSize;
	byte8* messageData;
};
