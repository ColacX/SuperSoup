#pragma once

#include <Box2D\Box2D.h>

#include "Message.hpp"
#include "Object.h"

class Entity
{
public:
	b2Body* body;
	
	uint32 entityID;
	float32 positionX;
	float32 positionY;
	float32 rotation;
	float32 shapeWidth;
	float32 shapeHeight;
	float32 aftcX;
	float32 aftcY;
	enum BodyType{ dynamic_body, static_body } bodyType;

	Entity();
	~Entity();

	virtual void construct(b2World& worlds);
	virtual void destruct();

	virtual void run();
	virtual void draw();

	virtual Message getSync() const;
	virtual void setSync(const Message& message);
	virtual void setSync2(const Message& message);
	virtual void setSync3(const Message& message);
};