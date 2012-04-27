#include <string.h>

#include "MessageSystem.h"

#include "SharedMisc.hpp"

//intialize static array
const unsigned int M::struct_size[256] = 
{
	sizeof(M_EntityAssign),
	sizeof(M_EntityCreate),
	sizeof(M_EntityForce),
	sizeof(M_EntitySync),
	sizeof(M_GameChecksum),		
	sizeof(M_GameFrame),
	sizeof(M_GameResync),
	sizeof(M_GameStep),
	sizeof(M_TextAll)
};

//uses the message id to determine what kind of message it is and returns a perfect clones
M* M::clone()
{
	M* m = 0;

	if(id == M::E_EntityAssign)
		m = new M_EntityAssign(*(M_EntityAssign*)this);
	else if(id == M::E_EntityCreate)
		m = new M_EntityCreate(*(M_EntityCreate*)this);
	else if(id == M::E_EntityForce)
		m = new M_EntityForce(*(M_EntityForce*)this);
	else if(id == M::E_EntitySync)
		m = new M_EntitySync(*(M_EntitySync*)this);
	else if(id == M::E_GameChecksum)
		m = new M_GameChecksum(*(M_GameChecksum*)this);
	else if(id == M::E_GameFrame)
		m = new M_GameFrame(*(M_GameFrame*)this);
	else if(id == M::E_GameResync)
		m = new M_GameResync(*(M_GameResync*)this);
	else if(id == M::E_GameStep)
		m = new M_GameStep(*(M_GameStep*)this);
	else if(id == M::E_TextAll)
		m = new M_TextAll(*(M_TextAll*)this);
	else
		throw "undefined message id";

	return m;
}

//---message constructors---

M_EntitySync::M_EntitySync()
{
	this->id = E_EntitySync;
}

M_EntityCreate::M_EntityCreate()
{
	this->id = E_EntityCreate;
}

M_GameChecksum::M_GameChecksum(unsigned int framecount, unsigned int checksum)
{
	this->id = E_GameChecksum;
	this->framecount = framecount;
	this->checksum = checksum;
}

M_EntityAssign::M_EntityAssign(unsigned int entityID)
{
	this->id = E_EntityAssign;
	this->entityID = entityID;
}

M_EntityForce::M_EntityForce(unsigned int entityID, float forceX, float forceY)
{
	this->id = E_EntityForce;
	this->entityID = entityID;
	this->forceX = forceX;
	this->forceY = forceY;
}

M_GameFrame::M_GameFrame(unsigned int framecount)
{
	this->id = E_GameFrame;
	this->framecount = framecount;
}

M_GameStep::M_GameStep()
{
	this->id = E_GameStep;
}

M_TextAll::M_TextAll(const char* text)
{
	this->id = E_TextAll;
	memset(this->text, 0, sizeof(this->text));
	memcpy(this->text, text, strlen(text)+1);
}

M_GameResync::M_GameResync()
{
	this->id = E_GameResync;
}