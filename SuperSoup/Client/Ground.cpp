#include "Ground.h"

std::vector< Object > objects;

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


void Ground::createWorld(){
	world_size_x = 400;
	world_size_y = 400;
	world = new unsigned char[world_size_x * world_size_y];
		
	for(int y=0; y<world_size_y; ++y)
		for(int x=0; x<world_size_x; ++x)
			world[x + y*world_size_x] = generate(x-world_size_x/2 , y-world_size_y/2);
}

Ground::Ground(b2World& world){
	sx = 32;
	sy = 32;

	count = 0;
	bodies = new Object[sx*sy];
	for(int y=0; y<sy; ++y)
		for(int x=0; x<sx; ++x)
			bodies[x+y*sy].createBox(world,b2Vec2(x,y),b2Vec2(0.5f,0.5f),0.0f,b2_staticBody);

	createWorld();
}
bool Ground::generate(int x, int y) const{
	if(y>=0)
		return false;

	if(x%3==0 || x%5==0 || y%3==0 || y%5==0)
		return false;


	return true;
}
bool Ground::isBlock(int x, int y) const{
	x += world_size_x/2;
	y += world_size_y/2;

	if(x<0 || y<0 || x>= world_size_x || x>= world_size_y)
		return false;

	return world[x + y*world_size_y] == 1;
}

void Ground::drawCube(float x, float y) const{
	b2Vec2 vertices[4];
	float extend = 0.5f;
	vertices[0] = b2Vec2(x - extend, y - extend);
	vertices[1] = b2Vec2(x + extend, y - extend);
	vertices[2] = b2Vec2(x + extend, y + extend);
	vertices[3] = b2Vec2(x - extend, y + extend);

	b2Color color(
		unsigned int(47*x+13*y)%47 / 46.0f,
		unsigned int(73*x+91*y)%37 / 36.0f,
		unsigned int(51*x+11*y)%97 / 96.0f
	);

	void DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color);
	DrawPolygon( vertices, 4, color );
}

void Ground::test(){

}

void Ground::calc(float cx, float cy){
	b2Filter collisionDisabled;
	b2Filter collisionEnabled;
	collisionDisabled.maskBits =  0;
	collisionEnabled.maskBits  = ~0;

	count = 0;
	int e = 1;
	for(int y=-sy/2; y<=sy/2+1*e; y+=e){
		for(int x=-sx/2; x<=sx/2+1*e; x+=e){
			float bx = cx + x;
			float by = cy + y;
			int bxi = e*floor(bx/e);
			int byi = e*floor(by/e);
			

			if( isBlock(bxi,byi) ){
				//bodies[bxi+byi*sy].body->GetFixtureList()[0].SetFilterData(collisionEnabled);
				//drawCube(bxi,byi);
				bodies[count].body->SetTransform(b2Vec2(bxi,byi),0.0f);
				++count;
			}
			//else
			//	bodies[bxi+byi*sy].body->GetFixtureList()[0].SetFilterData(collisionDisabled);
		}
	}
	for(int i=0; i<count; ++i)
		bodies[i].body->GetFixtureList()[0].SetFilterData(collisionEnabled);
	for(int i=count; i<sx*sy; ++i)
		bodies[i].body->GetFixtureList()[0].SetFilterData(collisionDisabled);
}

void Ground::draw(){
	for(int i=0; i<count; ++i)
		drawCube(bodies[i].body->GetPosition().x,bodies[i].body->GetPosition().y);
		//bodies[i].DebugDrawBox();
}


void Ground::doMath(Object* o, float cx, float cy){
	if( o->body->GetPosition().x > cx + sx/2 || o->body->GetPosition().x < cx - sx/2 ||
		o->body->GetPosition().y > cy + sy/2 || o->body->GetPosition().y < cy - sy/2 )
		o->body->SetAwake(false);
	else
		o->body->SetAwake(true);
}

// CamereraPostionX, CameraPositionY
//void Ground::draw(float cx, float cy) const{
//	int e = 2.0f;
//	for(int y=-sy/2; y<=sy/2+1*e; y+=e){
//		for(int x=-sx/2; x<=sx/2+1*e; x+=e){
//			float bx = cx + x;
//			float by = cy + y;
//			int bxi = e*floor(bx/e);
//			int byi = e*floor(by/e);
//			
//			//b2Filter collisionDisabled;
//			//b2Filter collisionEnabled;
//			//collisionDisabled.maskBits =  0;
//			//collisionEnabled.maskBits  = ~0;

//			if( isBlock(bxi,byi) ){
//				//bodies[bxi+byi*sy].body->GetFixtureList()[0].SetFilterData(collisionEnabled);
//				drawCube(bxi,byi);
//			}
//			//else
//			//	bodies[bxi+byi*sy].body->GetFixtureList()[0].SetFilterData(collisionDisabled);
//		}
//	}
//}


void MyContactListener::BeginContact(b2Contact* contact)
{ /* handle begin event */ 
	b2Body* a = contact->GetFixtureA()->GetBody();
	b2Body* b = contact->GetFixtureB()->GetBody();
	bool playerIsOneOfTheObjects;
	if(a==player)
		playerIsOneOfTheObjects = true;
	else if(b==player){
		playerIsOneOfTheObjects = true;
		b2Body* c = a;
		a = b;
		b = c;
	}
	if(playerIsOneOfTheObjects){
		// a = player nu, b den andra
		//b->SetAwake(false);
		b->SetGravityScale(0.0f);
	}
}
void MyContactListener::EndContact(b2Contact* contact)
{ /* handle end event */ }
void MyContactListener::PreSolve(b2Contact* contact, const b2Manifold* oldManifold)
{ /* handle pre-solve event */ }
void MyContactListener::PostSolve(b2Contact* contact, const b2ContactImpulse* impulse)
{ /* handle post-solve event */ }