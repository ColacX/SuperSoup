#pragma comment(lib, "Box2D.lib" ) //box2d physics engine library
#pragma comment(lib, "opengl32.lib") //opengl library for 32-bit computers
#pragma comment(lib, "glu32.lib") //openglu32 librarary extension to opengl

#include <Windows.h>
#include <gl\GL.h>

#include "Entity.h"

#include "SharedMisc.hpp"

Entity::Entity()
{
	entityID = (uint32)this;
	positionX = 0.0f;
	positionY = 0.0f;
	shapeWidth = 0.5f;
	shapeHeight = 0.5f;
	bodyType = dynamic_body;
}

Entity::~Entity()
{
}

void Entity::construct(b2World& world)
{
	b2Vec2 pos = b2Vec2_zero;
	float32 angle = 0.0f;
	b2Color color = b2Color( 0.5f +(0.5f*rand())/RAND_MAX,0.5f+(0.5f*rand())/RAND_MAX,0.5f+(0.5f*rand())/RAND_MAX);

	b2PolygonShape playerShape;
	playerShape.SetAsBox(shapeWidth, shapeHeight);

	//body def
	b2BodyDef bodyDef;

	if(bodyType == static_body)
		bodyDef.type = b2_staticBody;
	else if(bodyType == dynamic_body)
		bodyDef.type = b2_dynamicBody;
	else
		throw "error invalid body def types";

	bodyDef.position.x = positionX;
	bodyDef.position.y = positionY;
	bodyDef.angle = angle;

	//body
	// Call the body factory which allocates memory for the ground body
	// from a pool and creates the ground box shape (also from a pool).
	// The body is also added to the world.
	body = world.CreateBody(&bodyDef);

	//fixture def
	b2FixtureDef playerFixture;
	playerFixture.shape = &playerShape;
	playerFixture.restitution = 0.5f;	// air resistance / fluid resistance
	playerFixture.density = 1.0f;
	playerFixture.friction = 5.0f;

	//fixture
	body->CreateFixture(&playerFixture);

	b2MassData md;
	md.center = b2Vec2_zero;
	md.I = 8.0f/3.0f;
	md.mass = 1.0f;
	body->SetMassData(&md);
}

void Entity::destruct()
{
}

void Entity::run()
{
}

void DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
{
	glColor3f(color.r, color.g, color.b);
	glBegin(GL_LINE_LOOP);
	for (int32 i = 0; i < vertexCount; ++i)
	{
		glVertex2f(vertices[i].x, vertices[i].y);
	}
	glEnd();
}

void Entity::draw()
{
	b2Vec2 position = body->GetPosition();

	if(entityID == 1337)
	{
		printf("position: %f, %f\n", position.x, position.y);
	}

	glBegin(GL_POINTS);
	glVertex2f(position.x, position.y);
	glEnd();

	b2Color color;
	
	if( body->IsAwake() )
		color = b2Color(1,0,0);
	else
		color = b2Color(0,0,1);

	auto transform = body->GetTransform();
	b2Vec2 vertices[b2_maxPolygonVertices];
	
	for (b2Fixture* f = body->GetFixtureList(); f; f = f->GetNext())
	{
		b2PolygonShape* polygonShape = (b2PolygonShape*)f->GetShape();

		for(int32 i=0; i<polygonShape->m_vertexCount; i++)
			vertices[i] = b2Mul( transform, polygonShape->m_vertices[i] );

		DrawPolygon( vertices, polygonShape->m_vertexCount, color );
	}
}

Message Entity::getSync() const
{
	b2Vec2 position = body->GetPosition();
	
	Message message;
	message.recpientID = 23;
	message.messageSize = sizeof(uint32) + 4 * sizeof(float32) + sizeof(BodyType);
	message.messageData = new char[message.messageSize];

	unsigned int offset = 0;

	((uint32*)&message.messageData[offset])[0] = entityID;
	offset += sizeof(uint32);
	
	((float32*)&message.messageData[offset])[0] = position.x;
	offset += sizeof(float32);

	((float32*)&message.messageData[offset])[0] = position.y;
	offset += sizeof(float32);

	((float32*)&message.messageData[offset])[0] = shapeWidth;
	offset += sizeof(float32);

	((float32*)&message.messageData[offset])[0] = shapeHeight;
	offset += sizeof(float32);

	((uint32*)&message.messageData[offset])[0] = bodyType;
	offset += sizeof(uint32);

	//delete[] message.messageData; //tip if u screw this up use this line
	return message;
}

void Entity::setSync(const Message& message)
{
	if( message.messageSize != sizeof(uint32) + 4 * sizeof(float32) + sizeof(BodyType) )
		throw "bad sync message";

	unsigned int offset = 0;

	entityID = ((uint32*)&message.messageData[offset])[0];
	offset += sizeof(uint32);
	
	positionX = ((float32*)&message.messageData[offset])[0];
	offset += sizeof(float32);

	positionY = ((float32*)&message.messageData[offset])[0];
	offset += sizeof(float32);

	shapeWidth = ((float32*)&message.messageData[offset])[0];
	offset += sizeof(float32);

	shapeHeight = ((float32*)&message.messageData[offset])[0];
	offset += sizeof(float32);

	bodyType = (BodyType)((uint32*)&message.messageData[offset])[0];
	offset += sizeof(uint32);
}
