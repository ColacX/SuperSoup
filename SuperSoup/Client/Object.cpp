#include "Object.h"


#include "../shared/SharedMisc.hpp"



void Object::createBox( b2World& world,
				b2Vec2 pos,
				b2Vec2 size,
				float32 angle,
				b2BodyType objectType,
				b2Color color){
	this->color = color;

	b2PolygonShape playerShape;
	playerShape.SetAsBox(size.x, size.y);

	//body def
	b2BodyDef playerBodyDef;
	playerBodyDef.type = objectType;
	playerBodyDef.position = pos;
	playerBodyDef.angle = angle;

	//body
	// Call the body factory which allocates memory for the ground body
	// from a pool and creates the ground box shape (also from a pool).
	// The body is also added to the world.
	body = world.CreateBody(&playerBodyDef);

	//fixture def
	b2FixtureDef playerFixture;
	playerFixture.shape = &playerShape;
	playerFixture.restitution = 0.8f;	// air resistance / fluid resistance
	playerFixture.density = 1.0f;
	playerFixture.friction = 5.0f;

	//fixture
	body->CreateFixture(&playerFixture);
}

void Object::setMass( float mass ){
	b2MassData md;
	md.center = b2Vec2_zero;
	md.I = 2.6666667f;	// 8/3 is the default value. (This value will be returned when using GetMassData().
	md.mass = mass;
	body->SetMassData(&md);
}

void Object::DebugDrawBox()
{
	b2Color color;
	
	if( body->IsAwake() )
		color = this->color;
	else{
		float k = 0.25f;
		color = b2Color(0,0,1);
	}

	auto transform = body->GetTransform();
	b2Vec2 vertices[b2_maxPolygonVertices];
	
	for (b2Fixture* f = body->GetFixtureList(); f; f = f->GetNext())
	{
		b2PolygonShape* polygonShape = (b2PolygonShape*)f->GetShape();

		for(int32 i=0; i<polygonShape->m_vertexCount; i++)
			vertices[i] = b2Mul( transform, polygonShape->m_vertices[i] );

		void DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color);
		DrawPolygon( vertices, polygonShape->m_vertexCount, color );
	}
}