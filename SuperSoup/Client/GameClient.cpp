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
	gluOrtho2D(0, gameWidth/10.0f,-gameHeight/10.0f, 0);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	
	glTranslatef(+gameWidth/20.0f, -gameHeight/20.0f, 0);
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
					b2Vec2 size = b2Vec2(1.0f,1.0f),
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
		playerFixture.restitution = 0.8f;
		playerFixture.density = 1.0f;
		playerFixture.friction = 0.8f;

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
	bool operator[](const b2Vec2& p) const{
		if(p.y<0)
			return true;
		return false;
	}
	void draw(const b2Vec2& cameraPosition) const{

	}
};

void GameClient::run(){
	Ground g;
	bool b = g[b2Vec2(1.0f,2.0f)];

	// Define the gravity vector.
	b2Vec2 gravity(0.0f, -10.0f);

	// Construct a world object, which will hold and simulate the rigid bodies.
	b2World world(gravity);
	world.SetAllowSleeping(true);

	// Define the ground body.
	Object ground;
	ground.createBox(world,b2Vec2(0,-1),b2Vec2(100,1),0,b2_staticBody);
	objects.push_back(ground);
	
	for(float y=10.0f; y<=60.0f; y+=2.0f){
		for(float x=-70.0f; x<=70.0f; x+=2.0f){
			Object other;
			other.createBox(world,b2Vec2(x,y));
			objects.push_back(other);
		}
	}
	
	Object playerObj;
	playerObj.createBox(world,b2Vec2(0,30));
	objects.push_back(playerObj);

	player = playerObj.body;
	b2MassData md;
	md.center = b2Vec2_zero;
	md.I = 2.6666667f;
	md.mass = 40.0f;
	player->SetMassData(&md);
	
	// Prepare for simulation. Typically we use a time step of 1/60 of a
	// second (60Hz) and 10 iterations. This provides a high quality simulation
	// in most game scenarios.
	float32 timeStep = 1.0f / 60.0f;
	int32 velocityIterations = 1;//6;
	int32 positionIterations = 1;//2;

    //start window
    Window w0( false, "SuperSoup" );
    window0 = &w0;
    //window0->setSize( 400, 400 );
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
		world.Step(timeStep, velocityIterations, positionIterations);

		//------------ Camera trixing ------------		
		glLoadIdentity();
		glTranslatef(+gameWidth/20.0f, -gameHeight/20.0f, 0);
		glTranslatef(-player->GetPosition().x,-player->GetPosition().y,0.0f);
		printf("x:%f\ty:%f\n", player->GetPosition().x, player->GetPosition().y);

		//origo
		DrawPoint( b2Vec2(0,0), 3.0f, b2Color(0.0f,1.0f,0.0f));

		//DebugDrawBox(*body, dynamicBox);
		//DebugDrawBox(*groundBody, groundBox);

		//DebugDrawBox(*playerBody, playerShape);
		//o.DebugDrawBox();

		for(size_t o=0; o<objects.size(); ++o)
			objects[o].DebugDrawBox();

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
