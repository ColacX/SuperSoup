#pragma once

#include <Box2D\Box2D.h>

#include "MessageSystem.h"

class Entity
{
private:
	void fill(M_EntitySync& m);

public:
	b2Body* body;
	
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

	Entity();
	~Entity();

	virtual void construct(b2World& worlds);
	virtual void destruct();

	virtual void run();
	virtual void draw();

	M_EntityCreate* getCreate();
	M_EntitySync* getSync();
	void setSync(const M_EntitySync& m); //todo make constructor?
	void reSync(const M_EntitySync& m);
	M_EntityForce* getAFTC();
	void setAFTC(const M_EntityForce& m);
	
	uint32 getChecksum();
};