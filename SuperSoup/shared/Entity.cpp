#pragma comment(lib, "Box2D.lib" ) //box2d physics engine library
#pragma comment(lib, "opengl32.lib") //opengl library for 32-bit computers
#pragma comment(lib, "glu32.lib") //openglu32 librarary extension to opengl

#include <Windows.h>
#include <gl\GL.h>

#include "Entity.h"

#include "SharedMisc.hpp"

Entity::Entity()
{
	b2Body* body = 0;
	
	entityID = (uint32)this;
	bodyType = dynamic_body;
	shapeWidth = 0.5f;
	shapeHeight = 0.5f;
	aftcX = 0;
	aftcY = 0;

	angle = 0;
	angularDamping = 0;
	angularVelocity = 0;
	gravityScale = 1.0f;
	intertia = 0;
	linearDamping = 0;
	linearVelocityX = 0;
	linearVelocityY = 0;
	mass = 1.0f;
	positionX = 0;
	positionY = 0;

	isActive = true;
	isAwake = true;
	isBullet = false;
	isFixedRotation = false;
	isSleepingAllowed = true;
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

	

	body->SetActive(isActive);
	body->SetAngularDamping(angularDamping);
	body->SetAngularVelocity(angularVelocity);
	body->SetAwake(isAwake);
	body->SetBullet(isBullet);
	body->SetFixedRotation(isFixedRotation);
	body->SetGravityScale(gravityScale);
	body->SetLinearDamping(linearDamping);
	body->SetLinearVelocity( b2Vec2(linearVelocityX, linearVelocityY) );
	
	b2MassData md;
	md.center = b2Vec2_zero;
	md.I = 8.0f/3.0f;
	md.mass = mass;
	body->SetMassData(&md);

	body->SetSleepingAllowed(isSleepingAllowed);
	body->SetTransform( b2Vec2(positionX, positionY), angle);
	//body->SetType();
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
	float32 angle = body->GetAngle();
	float32 angularDamping = body->GetAngularDamping();
	float32 angularVelocity = body->GetAngularVelocity();
	float32 gravityScale = body->GetGravityScale();
	float32 intertia = body->GetInertia();
	float32 linearDamping = body->GetLinearDamping();
	b2Vec2 linearVelocity = body->GetLinearVelocity();
	//body->GetLinearVelocityFromLocalPoint();
	//body->GetLinearVelocityFromWorldPoint();
	//body->GetLocalCenter();
	//body->GetLocalPoint();
	//body->GetLocalVector();
	float32 mass = body->GetMass();
	b2Vec2 position = body->GetPosition();
	//body->GetTransform();
	bool isActive = body->IsActive();
	bool isAwake = body->IsAwake();
	bool isBullet = body->IsBullet();
	bool isFixedRotation = body->IsFixedRotation();
	bool isSleepingAllowed = body->IsSleepingAllowed();
	
	Message message;
	message.recpientID = entityID;
	message.messageSize = 2 * sizeof(uint32) + 15 * sizeof(float32) + 5 * sizeof(bool);
	message.messageData = new char[message.messageSize];

	unsigned int offset = 0;

	*((uint32*)&message.messageData[offset]) = entityID; offset += sizeof(uint32);
	*((uint32*)&message.messageData[offset]) = bodyType; offset += sizeof(uint32);

	*((float32*)&message.messageData[offset]) = shapeWidth; offset += sizeof(float32);
	*((float32*)&message.messageData[offset]) = shapeHeight; offset += sizeof(float32);
	*((float32*)&message.messageData[offset]) = aftcX; offset += sizeof(float32);
	*((float32*)&message.messageData[offset]) = aftcY; offset += sizeof(float32);

	*((float32*)&message.messageData[offset]) = angle; offset += sizeof(float32);
	*((float32*)&message.messageData[offset]) = angularDamping; offset += sizeof(float32);
	*((float32*)&message.messageData[offset]) = angularVelocity; offset += sizeof(float32);
	*((float32*)&message.messageData[offset]) = gravityScale; offset += sizeof(float32);
	*((float32*)&message.messageData[offset]) = intertia; offset += sizeof(float32);
	*((float32*)&message.messageData[offset]) = linearDamping; offset += sizeof(float32);
	*((float32*)&message.messageData[offset]) = linearVelocity.x; offset += sizeof(float32);
	*((float32*)&message.messageData[offset]) = linearVelocity.y; offset += sizeof(float32);
	*((float32*)&message.messageData[offset]) = mass; offset += sizeof(float32);
	*((float32*)&message.messageData[offset]) = position.x; offset += sizeof(float32);
	*((float32*)&message.messageData[offset]) = position.y; offset += sizeof(float32);
	
	*((bool*)&message.messageData[offset]) = isActive; offset += sizeof(bool);
	*((bool*)&message.messageData[offset]) = isAwake; offset += sizeof(bool);
	*((bool*)&message.messageData[offset]) = isBullet; offset += sizeof(bool);
	*((bool*)&message.messageData[offset]) = isFixedRotation; offset += sizeof(bool);
	*((bool*)&message.messageData[offset]) = isSleepingAllowed; offset += sizeof(bool);

	return message;
}

void Entity::setSync(const Message& message)
{
	if( message.messageSize != 2 * sizeof(uint32) + 15 * sizeof(float32) + 5 * sizeof(bool) )
		throw "bad sync message";

	unsigned int offset = 0;

	entityID = *((uint32*)&message.messageData[offset]); offset += sizeof(uint32);
	bodyType = (BodyType)*((uint32*)&message.messageData[offset]); offset += sizeof(uint32);

	shapeWidth = *((float32*)&message.messageData[offset]); offset += sizeof(float32);
	shapeHeight = *((float32*)&message.messageData[offset]); offset += sizeof(float32);
	aftcX = *((float32*)&message.messageData[offset]); offset += sizeof(float32);
	aftcY = *((float32*)&message.messageData[offset]); offset += sizeof(float32);

	angle = *((float32*)&message.messageData[offset]); offset += sizeof(float32);
	angularDamping = *((float32*)&message.messageData[offset]); offset += sizeof(float32);
	angularVelocity = *((float32*)&message.messageData[offset]); offset += sizeof(float32);
	gravityScale = *((float32*)&message.messageData[offset]); offset += sizeof(float32);
	intertia = *((float32*)&message.messageData[offset]); offset += sizeof(float32);
	linearDamping = *((float32*)&message.messageData[offset]); offset += sizeof(float32);
	linearVelocityX = *((float32*)&message.messageData[offset]); offset += sizeof(float32);
	linearVelocityY = *((float32*)&message.messageData[offset]); offset += sizeof(float32);
	mass = *((float32*)&message.messageData[offset]); offset += sizeof(float32);
	positionX = *((float32*)&message.messageData[offset]); offset += sizeof(float32);
	positionY = *((float32*)&message.messageData[offset]); offset += sizeof(float32);

	isActive = *((bool*)&message.messageData[offset]); offset += sizeof(bool);
	isAwake = *((bool*)&message.messageData[offset]); offset += sizeof(bool);
	isBullet = *((bool*)&message.messageData[offset]); offset += sizeof(bool);
	isFixedRotation = *((bool*)&message.messageData[offset]); offset += sizeof(bool);
	isSleepingAllowed = *((bool*)&message.messageData[offset]); offset += sizeof(bool);
}

Message Entity::getAFTC()
{
	Message message;
	message.recpientID = entityID;
	message.messageSize = 2 * sizeof(float32);
	message.messageData = new char[message.messageSize];

	unsigned int offset = 0;

	*((float32*)&message.messageData[offset]) = aftcX; offset += sizeof(float32);
	*((float32*)&message.messageData[offset]) = aftcY; offset += sizeof(float32);

	return message;
}

void Entity::setAFTC(const Message& message)
{
	if( message.messageSize != 2 * sizeof(float32) )
		throw "bad aftc message";

	unsigned int offset = 0;

	aftcX = *((float32*)&message.messageData[offset]); offset += sizeof(float32);
	aftcY = *((float32*)&message.messageData[offset]); offset += sizeof(float32);

	body->ApplyForceToCenter( b2Vec2(aftcX, aftcY) );
	printf("aftc x: %f y: %f\n", aftcX, aftcY);
	aftcX = 0;
	aftcY = 0;
}