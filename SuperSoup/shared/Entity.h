#pragma once

#include <Box2D\Box2D.h>

#include "Message.hpp"
#include "Object.h"

class Entity
{
public:
	enum BodyType{ dynamic_body, static_body };
	
	b2Body* body;
	
	uint32 entityID;
	BodyType bodyType;
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

	virtual Message getSync(bool print = false);
	virtual void setSync(const Message& message);
	virtual Message getAFTC();
	virtual void setAFTC(const Message& message);
	uint32 getChecksum();
};