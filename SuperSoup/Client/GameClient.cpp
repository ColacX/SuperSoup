#include "GameClient.h"

#include <Box2D\Box2D.h>
#include <cstdio>
#include <Box2D\Box2D.h>

void DrawTransform(const b2Transform& xf)
{
	b2Vec2 p1 = xf.p, p2;
	const float32 k_axisScale = 0.4f;
	glBegin(GL_LINES);
	
	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex2f(p1.x, p1.y);
	p2 = p1 + k_axisScale * xf.q.GetXAxis();
	glVertex2f(p2.x, p2.y);

	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex2f(p1.x, p1.y);
	p2 = p1 + k_axisScale * xf.q.GetYAxis();
	glVertex2f(p2.x, p2.y);

	glEnd();
}

void DrawPoint(const b2Vec2& p, float32 size, const b2Color& color)
{
	glPointSize(size);
	glBegin(GL_POINTS);
	glColor3f(color.r, color.g, color.b);
	glVertex2f(p.x, p.y);
	glEnd();
	glPointSize(1.0f);
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

void DebugDrawBox(const b2Body& body, const b2PolygonShape& polygonShape)
{
	b2Color color;
	
	if( body.IsAwake() )
		color = b2Color(1,0,0);
	else
		color = b2Color(0,0,1);

	auto transform = body.GetTransform();
	b2Vec2 vertices[b2_maxPolygonVertices];
	
	for(int32 i=0; i<polygonShape.m_vertexCount; i++)
	{
		vertices[i] = b2Mul( transform, polygonShape.m_vertices[i] );
	}

	DrawPolygon( vertices, polygonShape.m_vertexCount, color );
}

GameClient::GameClient(){
    isRunning = false;
    framecount = 0;
    ZeroMemory( keydown, sizeof(keydown) );
    window0 = 0;
    ingame = false;
}
GameClient::~GameClient(){
    isRunning = false;
    //wait for run to stop?
}

void GameClient::setVerticalSync(bool sync){
    sync;
    typedef bool (APIENTRY *PFNWGLSWAPINTERVALFARPROC)(int);
    PFNWGLSWAPINTERVALFARPROC wglSwapIntervalEXT = 0;
    wglSwapIntervalEXT = (PFNWGLSWAPINTERVALFARPROC)wglGetProcAddress("wglSwapIntervalEXT");
    wglSwapIntervalEXT(sync);
}

void GameClient::windowClosed(){
    isRunning = false;
}

void GameClient::windowResized(int width, int height)
{
	gameWidth = width;
	gameHeight = height;

	glViewport(0, 0, gameWidth, height);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, gameWidth/20.0f,-gameHeight/20.0f, 0);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	
	glTranslatef(+gameWidth/40.0f, -gameHeight/40.0f, 0);
}

void GameClient::windowUnfocused(){
    while( ShowCursor(true)<0 ){}
    this->mouseCenterX = 0;
    this->mouseCenterY = 0;
    ZeroMemory(this->keydown,256);
}

void GameClient::keyPressed(unsigned int virtualKeyCode){
    this->keydown[virtualKeyCode] = true;
}
void GameClient::keyReleased(unsigned int virtualKeyCode){
    this->keydown[virtualKeyCode] = false;
}

void GameClient::mousePressed( unsigned int button, int localX, int localY){
}

struct Object{
	b2Body* body;
	b2Color color;
	void createBox( b2World& world,
					b2Vec2 pos = b2Vec2_zero,
					b2Vec2 size = b2Vec2(0.5f,0.5f),
					float32 angle = 0.0f,
					b2BodyType objectType = b2_dynamicBody,
					b2Color color = b2Color(0.5f+(0.5f*rand())/RAND_MAX,0.5f+(0.5f*rand())/RAND_MAX,0.5f+(0.5f*rand())/RAND_MAX) ){
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

	void DebugDrawBox()
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

			DrawPolygon( vertices, polygonShape->m_vertexCount, color );
		}
	}
};
#include <vector>
std::vector< Object > objects;

class Ground{
public:
	int sx;
	int sy;

	int oldPx;
	int oldPy;

	Object* bodies; // [sy][sx]
	size_t count;

	Ground(b2World& world){
		sx = 64;
		sy = 64;

		count = 0;
		bodies = new Object[sx*sy];
		for(int y=0; y<sy; ++y)
			for(int x=0; x<sx; ++x)
				bodies[x+y*sy].createBox(world,b2Vec2(x,y),b2Vec2(0.5f,0.5f),0.0f,b2_staticBody);
	}
	bool isBlock(int x, int y) const{
		if(y>=0)
			return false;

		if(x%3==0 || x%5==0 || y%3==0 || y%5==0)
			return false;


		return true;
	}
	void drawCube(float x, float y) const{
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

		DrawPolygon( vertices, 4, color );
	}

	void test(){

	}

	void calc(float cx, float cy){
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

	void draw(){
		for(int i=0; i<count; ++i)
			drawCube(bodies[i].body->GetPosition().x,bodies[i].body->GetPosition().y);
			//bodies[i].DebugDrawBox();
	}

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

void GameClient::run(){
	//B2_NOT_USED(argc);
	//B2_NOT_USED(argv);

	// Define the gravity vector.
	b2Vec2 gravity(0.0f, -10.0f);

	// Construct a world object, which will hold and simulate the rigid bodies.
	b2World world(gravity);
	world.SetAllowSleeping(true);

	// Define the ground body.
	Object ground;
	ground.createBox(world,b2Vec2(0,-1),b2Vec2(100,1),0,b2_staticBody);
	objects.push_back(ground);
	
	size_t antal = 0;
	for(float y=10.0f; y<=30.0f; y+=1.0f){
		for(float x=-20.0f; x<=20.0f; x+=1.0f){
			Object other;
			other.createBox(world,b2Vec2(x,y));
			objects.push_back(other);
			++antal;
		}
	}
	printf( "\nantal boxar som skapas = %u\n\n", antal );

	
	Ground g(world);
	//bool b = g.isBlock(b2Vec2(1.0f,2.0f));
	
	Object playerObj;
	playerObj.createBox(world,b2Vec2(0,30));
	objects.push_back(playerObj);

	player = playerObj.body;
	b2MassData md;
	md.center = b2Vec2_zero;
	md.I = 2.6666667f;	// 8/3 is the default value.
	md.mass = 40.0f;
	player->SetMassData(&md);
	
	// Prepare for simulation. Typically we use a time step of 1/60 of a
	// second (60Hz) and 10 iterations. This provides a high quality simulation
	// in most game scenarios.
	float32 timeStep = 1.0f / 60.0f;
	int32 velocityIterations = 6;
	int32 positionIterations = 2;

    //start window
    Window w0( false, "SuperSoup" );
    window0 = &w0;
    //window0->setSize( 1920, 1080 );
    window0->addWindowListener( this );
    window0->addKeyboardListener( this );
    window0->addMouseListener( this );
    window0->create();
    window0->setMaximized();

    //get device context and rendering context
    HDC windowDeviceContext0 = window0->getDeviceContext();
    HGLRC renderingContext0 = wglCreateContext( windowDeviceContext0 );

    //this thread must own the gl rendering context or gl calls will be ignored
    BOOL rwglMakeCurrent = wglMakeCurrent( windowDeviceContext0, renderingContext0);
    
	if(!rwglMakeCurrent)
        throw "GameClient: wglMakeCurrent failed";

    setVerticalSync(true);
	
	glHint(GL_LINE_SMOOTH, GL_NICEST);
	glEnable(GL_LINE_SMOOTH);

	windowResized(window0->getWidth(), window0->getHeight() );
	isRunning = true;

    while(isRunning)
	{
        //check user interactions
        for(int i=0; i<5; i++)
            window0->run();

        this->checkControls();

		//reset drawing buffer
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		
		ground.body->SetAwake(false);
		//player->ApplyForceToCenter(b2Vec2(0.0f, 60 * 40.0f * timeStep));
		//draw body stuff
		g.calc(player->GetPosition().x, player->GetPosition().y);
		world.Step(timeStep, velocityIterations, positionIterations);

		//------------ Camera trixing ------------		
		glLoadIdentity();
		glTranslatef(+gameWidth/40.0f, -gameHeight/40.0f, 0);
		glTranslatef(-player->GetPosition().x,-player->GetPosition().y,0.0f);
		//printf("x:%f\ty:%f\n", player->GetPosition().x, player->GetPosition().y);

		//origo
		DrawPoint( b2Vec2(0,0), 3.0f, b2Color(0.0f,1.0f,0.0f));

		//DebugDrawBox(*body, dynamicBox);
		//DebugDrawBox(*groundBody, groundBox);

		//DebugDrawBox(*playerBody, playerShape);
		//o.DebugDrawBox();

		g.draw();

		for(size_t o=0; o<objects.size(); ++o)
			objects[o].DebugDrawBox();

		//g.draw(player->GetPosition().x,player->GetPosition().y);

        window0->swapBuffers();

        //sleep //some kind of syncing is necessary or the computer will crash
        //Sleep(1000/60);
        framecount++;
    }

    printf("render loop end\n");
}

void GameClient::checkControls(){
    
    //special keys
    if(keydown[VK_ESCAPE]){
        windowUnfocused();
        ingame = false;
    }
	
	const float forceConstant = 60.0f * player->GetMass();
	if(keydown[VK_UP])
		player->ApplyForceToCenter(b2Vec2(0.0f,forceConstant));
	if(keydown[VK_DOWN])
		player->ApplyForceToCenter(b2Vec2(0.0f,-forceConstant));
	if(keydown[VK_LEFT])
		player->ApplyForceToCenter(b2Vec2(-forceConstant,0.0f));
	if(keydown[VK_RIGHT])
		player->ApplyForceToCenter(b2Vec2(forceConstant,0.0f));

}
