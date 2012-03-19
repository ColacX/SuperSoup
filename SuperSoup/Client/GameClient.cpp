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
    if( button == MouseListener::BUTTON_LEFT){
        if( !ingame ){
            ingame = true;
            POINT point;
            GetCursorPos(&point);
            this->mouseCenterX = point.x; //global X
            this->mouseCenterY = point.y; //global y

            while(ShowCursor(false)>-1){}
        }
    }
}

void GameClient::run(){
	//B2_NOT_USED(argc);
	//B2_NOT_USED(argv);

	// Define the gravity vector.
	b2Vec2 gravity(0.0f, -10.0f);

	// Construct a world object, which will hold and simulate the rigid bodies.
	b2World world(gravity);
	world.SetAllowSleeping(true);

	// Define the ground body.
	b2BodyDef groundBodyDef;
	groundBodyDef.position.Set(0.0f, -1.0f);

	// Call the body factory which allocates memory for the ground body
	// from a pool and creates the ground box shape (also from a pool).
	// The body is also added to the world.
	b2Body* groundBody = world.CreateBody(&groundBodyDef);

	// Define the ground box shape.
	b2PolygonShape groundBox;

	// The extents are the half-widths of the box.
	groundBox.SetAsBox(100.0f, 1.0f);

	// Add the ground fixture to the ground body.
	groundBody->CreateFixture(&groundBox, 0.0f);

	// Define the dynamic body. We set its position and call the body factory.
	b2BodyDef bodyDef;
	bodyDef.type = b2_dynamicBody;
	bodyDef.position.Set(0.0f, 8.0f);
	bodyDef.fixedRotation = false;
	bodyDef.angle = 1.0f;
	b2Body* body = world.CreateBody(&bodyDef);

	// Define another box shape for our dynamic body.
	b2PolygonShape dynamicBox;
	dynamicBox.SetAsBox(1.0f, 1.0f);

	// Define the dynamic body fixture.
	b2FixtureDef fixtureDef;
	fixtureDef.shape = &dynamicBox;

	//bouncy ness
	fixtureDef.restitution = 0.8f;

	// Set the box density to be non-zero, so it will be dynamic.
	fixtureDef.density = 1.0f;

	// Override the default friction.
	fixtureDef.friction = 0.8f;

	// Add the shape to the body.
	body->CreateFixture(&fixtureDef);

	// Prepare for simulation. Typically we use a time step of 1/60 of a
	// second (60Hz) and 10 iterations. This provides a high quality simulation
	// in most game scenarios.
	float32 timeStep = 1.0f / 60.0f;
	int32 velocityIterations = 6;
	int32 positionIterations = 2;

	//----------------player------------------------------

	//shape
	b2PolygonShape playerShape;
	playerShape.SetAsBox(1.0f, 1.0f);

	//body def
	b2BodyDef playerBodyDef;
	playerBodyDef.type = b2_dynamicBody;
	playerBodyDef.position.Set(3.0f, 8.0f);
	playerBodyDef.fixedRotation = false;
	playerBodyDef.angle = 1.0f;

	//body
	b2Body* playerBody = world.CreateBody(&playerBodyDef);

	//fixture def
	b2FixtureDef playerFixture;
	playerFixture.shape = &playerShape;
	playerFixture.restitution = 0.8f;
	playerFixture.density = 1.0f;
	playerFixture.friction = 0.8f;

	//fixture
	playerBody->CreateFixture(&playerFixture);

	player = playerBody;

	// When the world destructor is called, all bodies and joints are freed. This can
	// create orphaned pointers, so be careful about your world management.

    //important thread required initiation
    //some functions and methods must be called within this thread or they will be ignored
    //because this is the render context onwning thread
    //BOOL b = SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_HIGHEST); //sets thread priority

    //start window
    Window w0( false, "SuperSoup" );
    window0 = &w0;
    //window0->setSize( 400, 400 );
    window0->addWindowListener( this );
    window0->addKeyboardListener( this );
    window0->addMouseListener( this );
    window0->create();
    //window0->setMaximized();

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

		//draw body stuff
		// This is our little game loop.
		// Instruct the world to perform a single step of simulation.
		// It is generally best to keep the time step and iterations fixed.
		world.Step(timeStep*4.0f, velocityIterations, positionIterations);

		//------------ Camera trixing ------------		
		glLoadIdentity();
		glTranslatef(+gameWidth/20.0f, -gameHeight/20.0f, 0);
		glTranslatef(-player->GetPosition().x,-player->GetPosition().y,0.0f);
		printf("x:%f\ty:%f\n", player->GetPosition().x, player->GetPosition().y);

		//origo
		DrawPoint( b2Vec2(0,0), 3.0f, b2Color(0.0f,1.0f,0.0f));

		DebugDrawBox(*body, dynamicBox);
		DebugDrawBox(*groundBody, groundBox);

		DebugDrawBox(*playerBody, playerShape);

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
	
	const float forceConstant = 60.0f;
	if(keydown[VK_UP])
		player->ApplyForceToCenter(b2Vec2(0.0f,forceConstant));
	if(keydown[VK_DOWN])
		player->ApplyForceToCenter(b2Vec2(0.0f,-forceConstant));
	if(keydown[VK_LEFT])
		player->ApplyForceToCenter(b2Vec2(-forceConstant,0.0f));
	if(keydown[VK_RIGHT])
		player->ApplyForceToCenter(b2Vec2(forceConstant,0.0f));

}
