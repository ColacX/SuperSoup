#pragma once

#include <Box2D\Box2D.h>

struct Object{
	b2Body* body;
	b2Color color;
	void createBox( b2World& world,
					b2Vec2 pos = b2Vec2_zero,
					b2Vec2 size = b2Vec2(0.5f,0.5f),
					float32 angle = 0.0f,
					b2BodyType objectType = b2_dynamicBody,
					b2Color color = b2Color(0.5f+(0.5f*rand())/RAND_MAX,0.5f+(0.5f*rand())/RAND_MAX,0.5f+(0.5f*rand())/RAND_MAX) );
	
	void setMass( float mass );

	void DebugDrawBox();
};