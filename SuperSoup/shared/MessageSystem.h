#pragma once

//message base class
struct M
{
	//identifiers for all message structs
	enum
	{
		E_EntityAssign,
		E_EntityCreate,
		E_EntityForce,
		E_EntitySync,
		E_GameChecksum,
		E_GameFrame,
		E_GameResync,
		E_GameStep,
		E_TextAll //EC1024_TextAll
	};

	//a collections of sizes for all message structs
	static const unsigned int struct_size[256];

	//todo fix real types?
	typedef unsigned char uchar8;
	typedef unsigned int uint32;
	typedef float float32;

	//message identifier
	uchar8 id;

	M* clone();
};

//synchronize existing entity
struct M_EntitySync : M
{
	uint32 entityID;
	uint32 bodyType;
	float32 shapeWidth;
	float32 shapeHeight;
	float32 aftcX;
	float32 aftcY;

	float32 angle;
	float32 angularDamping;
	float32 angularVelocity;
	float32 gravityScale;
	float32 intertia;
	float32 linearDamping;
	float32 linearVelocityX;
	float32 linearVelocityY;
	float32 mass;
	float32 positionX;
	float32 positionY;

	bool isActive;
	bool isAwake;
	bool isBullet;
	bool isFixedRotation;
	bool isSleepingAllowed;

	M_EntitySync();
};

//create entity
struct M_EntityCreate : M_EntitySync
{
	M_EntityCreate();
};

//checksum for all entitys in a certain frame, used for game sync
struct M_GameChecksum : M
{
	uint32 framecount;
	uint32 checksum;

	M_GameChecksum(unsigned int framecount, unsigned int checksum);
};

//assign a entity certain role
struct M_EntityAssign : M
{
	uint32 entityID;

	M_EntityAssign(unsigned int entityID);
};

//apply force to entity
struct M_EntityForce : M
{
	uint32 entityID;
	float32 forceX;
	float32 forceY;

	M_EntityForce(unsigned int entityID, float forceX, float forceY);
};

//syncs the game frame with the client
struct M_GameFrame : M
{
	uint32 framecount;

	M_GameFrame(unsigned int framecount);
};

//allow the game to step to next frame
struct M_GameStep : M
{
	M_GameStep();
};

//text message to all
struct M_TextAll : M
{
	uchar8 text[1024];

	M_TextAll(const char* text);
};

//requst resync game
struct M_GameResync : M
{
	M_GameResync();
};
