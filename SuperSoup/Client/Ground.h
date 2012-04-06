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

	void DebugDrawBox();
};
#include <vector>
extern std::vector< Object > objects;


class Ground{
public:
	int sx;
	int sy;

	int oldPx;
	int oldPy;

	Object* bodies; // [sy][sx]
	size_t count;

	int world_size_x;
	int world_size_y;
	unsigned char* world;

	void createWorld();

	Ground(b2World& world);
	bool generate(int x, int y) const;
	bool isBlock(int x, int y) const;
	void add(int x, int y);
	void del(int x, int y);

	static void drawCube(float x, float y);

	void test();

	void calc(float cx, float cy);

	void draw();


	void doMath(Object* o, float cx, float cy);

	// CamereraPostionX, CameraPositionY
	//void draw(float cx, float cy) const{
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
};

class MyContactListener : public b2ContactListener{
public:
	Ground* ground;
	b2Body* player;

	void BeginContact(b2Contact* contact);
	void EndContact(b2Contact* contact);
	void PreSolve(b2Contact* contact, const b2Manifold* oldManifold);
	void PostSolve(b2Contact* contact, const b2ContactImpulse* impulse);
};