#include <Box2D\Box2D.h>
#include <cstdio>
#include <WinSock2.h>

#include "GameClient.h"

#include "..\shared\Client.hpp"
#include "..\shared\Entity.h"

#include "..\shared\SharedMisc.hpp"

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

	void DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color);
	DrawPolygon( vertices, polygonShape.m_vertexCount, color );
}

GameClient::GameClient(){
    isRunning = false;
    clientFramecount = 0;
    ZeroMemory( keydown, sizeof(keydown) );
    window0 = 0;
    ingame = false;
	playerXXX = 0;
	clientXXX = 0;
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

int round(float f){
    return (int)(f < 0 ? (f - 0.5) : (f + 0.5));
}

void GameClient::mousePressed( unsigned int button, int localX, int localY){
	/*
	
	mx = localX;
	my = localY;

	
	float k = 20.0f;
	float mxInWorld = player->GetPosition().x + (mx-int(window0->getWidth()) /2)/k;
	float myInWorld = player->GetPosition().y - (my-int(window0->getHeight())/2)/k;

	
	if(button == MouseListener::BUTTON_LEFT)
		ground->add(round(mxInWorld),round(myInWorld));
	else if(button == MouseListener::BUTTON_RIGHT)
		ground->del(round(mxInWorld),round(myInWorld));
	*/
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
	Object groundObj;
	groundObj.createBox(world,b2Vec2(0,-1),b2Vec2(100,1),0,b2_staticBody);
	ground->objects.push_back(groundObj);
	
	size_t antal = 0;/*
	for(float y=10.0f; y<=30.0f/2; y+=1.0f){
		for(float x=-20.0f/2; x<=20.0f/2; x+=1.0f){
			Object other;
			other.createBox(world,b2Vec2(x,y));
			objects.push_back(other);
			++antal;
		}
	}*/
	printf( "\nantal boxar som skapas = %u\n\n", antal );

	
	ground = new Ground(world);
	//bool b = g.isBlock(b2Vec2(1.0f,2.0f));
	
	Object playerObj;
	playerObj.createBox(world,b2Vec2(0,30));
	ground->objects.push_back(playerObj);

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
        throw "wglMakeCurrent failed";

    setVerticalSync(true);
	
	glHint(GL_LINE_SMOOTH, GL_NICEST);
	glEnable(GL_LINE_SMOOTH);

	windowResized(window0->getWidth(), window0->getHeight() );
	isRunning = true;

	MyContactListener myContactListener;
	myContactListener.ground = ground;
	myContactListener.player = player;
	world.SetContactListener(&myContactListener);

    while(isRunning)
	{
        //check user interactions
        for(int i=0; i<5; i++)
            window0->run();

        this->checkControls();

		//reset drawing buffer
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		
		groundObj.body->SetAwake(false);
		//player->ApplyForceToCenter(b2Vec2(0.0f, 60 * 40.0f * timeStep));
		//draw body stuff
		ground->calc(player->GetPosition().x, player->GetPosition().y);

		world.Step(timeStep, velocityIterations, positionIterations);
		
		for(size_t o=0; o<ground->objects.size(); ++o)
			ground->doMath(&ground->objects[o],player->GetPosition().x,player->GetPosition().y);
		
		//------------ Camera trixing ------------		
		glLoadIdentity();
		glTranslatef(+gameWidth/40.0f, -gameHeight/40.0f, 0);
		glTranslatef(-player->GetPosition().x,-player->GetPosition().y,0.0f);
		//printf("x:%+e\ty:%+e\n", player->GetPosition().x, player->GetPosition().y);

		//origo
		DrawPoint( b2Vec2(0,0), 3.0f, b2Color(0.0f,1.0f,0.0f));

		//DebugDrawBox(*body, dynamicBox);
		//DebugDrawBox(*groundBody, groundBox);

		//DebugDrawBox(*playerBody, playerShape);
		//o.DebugDrawBox();

		ground->draw();

		for(size_t o=0; o<ground->objects.size(); ++o)
			ground->objects[o].DebugDrawBox();
		
		float k = 20.0f;
		float mxInWorld = player->GetPosition().x + (mx-int(window0->getWidth()) /2)/k;
		float myInWorld = player->GetPosition().y - (my-int(window0->getHeight())/2)/k;
		Ground::drawCube(mxInWorld,myInWorld);
		//ground->del(floor(mxInWorld),floor(myInWorld));
		

		//g.draw(player->GetPosition().x,player->GetPosition().y);

        window0->swapBuffers();

        //sleep //some kind of syncing is necessary or the computer will crash
        //Sleep(1000/60);
        clientFramecount++;
    }

    printf("render loop end\n");
}

void GameClient::run2()
{
	// Prepare for simulation. Typically we use a time step of 1/60 of a
	// second (60Hz) and 10 iterations. This provides a high quality simulation
	// in most game scenarios.
	float32 timeStep = 1.0f / 60.0f;
	int32 velocityIterations = 6;
	int32 positionIterations = 2;

	//world
	b2Vec2 gravity(0.0f, -10.0f);
	b2World world(gravity);
	world.SetAllowSleeping(true);

    //start window
    Window w0( false, "Client" );
    window0 = &w0;
    //window0->setSize( 1920, 1080 );
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
        throw "wglMakeCurrent failed";

    //setVerticalSync(true);
	
	glHint(GL_LINE_SMOOTH, GL_NICEST);
	glEnable(GL_LINE_SMOOTH);

	windowResized(window0->getWidth(), window0->getHeight() );
	isRunning = true;

	printf("client accepting process begin\n");

	Client client;
	clientXXX = &client;
	client.construct( Client::connectTo("127.0.0.1", "12000") );

	std::list<Entity*> listEntity;
	uint32 clientChecksum = 0;
	std::list<Pair<uint32, uint32>> listChecksum;

    while(isRunning)
	{
		//pull network messages to lists
        client.pushMessages();
		
		while(client.listMessage.size() > 0 )
		{
			Message message = client.listMessage.front();
			client.listMessage.pop_front();

			if(message.recpientID == 0)
			{
				//run game simulations
				world.Step(timeStep, velocityIterations, positionIterations);

				for(auto it = listEntity.begin(); it != listEntity.end(); it++)
				{
					Entity* entity = *it;
					entity->run();
				}

				clientFramecount++;
				clientChecksum = clientFramecount;

				for(auto it = listEntity.begin(); it != listEntity.end(); it++)
				{
					Entity* entity = *it;
					clientChecksum += entity->getChecksum();
				}

				Pair<uint32, uint32> p;
				p.a = clientFramecount;
				p.b = clientChecksum;
				listChecksum.push_back(p);

				//good for debugging
				printf("frame: %d\n", clientFramecount);

				for(auto itentity = listEntity.begin(); itentity != listEntity.end(); itentity++)
				{
					Entity* entity = *itentity;
					printf("XXXXX %d\n", clientFramecount);
					entity->getSync(true);
				}
			}
			else if(message.recpientID == 1)
			{
				//print a message
				printf("%d: %s\n", message.recpientID, message.messageData);
			}
			else if(message.recpientID == 2)
			{
				//create new entity
				Entity* entity = new Entity();
				entity->setSync(message);
				entity->construct(world);
				listEntity.push_back(entity);

				entity->getSync(true);
			}
			else if(message.recpientID == 3)
			{
				//create and set player entity
				Entity* entity = new Entity();
				entity->setSync(message);
				entity->construct(world);
				listEntity.push_back(entity);
				playerXXX = entity;

				entity->getSync(true);
			}
			else if(message.recpientID == 4)
			{
				if( message.messageSize != sizeof(uint32) )
					throw "bad clientFramecount message size";

				unsigned int offset = 0;

				uint32 serverFramecount = *((uint32*)&message.messageData[offset]); offset += sizeof(uint32);
				clientFramecount = serverFramecount;
				printf("clientFramecount set from server: %d\n", clientFramecount);
			}
			else if(message.recpientID == 5)
			{
				//compare with checksum
				if( message.messageSize != 2 * sizeof(uint32) )
					throw "bad checksum message size";

				unsigned int offset = 0;

				uint32 serverFramecount = *((uint32*)&message.messageData[offset]); offset += sizeof(uint32);
				uint32 serverChecksum = *((uint32*)&message.messageData[offset]); offset += sizeof(uint32);

				//printf("sfc: %d scs: %d\n", serverFramecount, serverChecksum);
					
				for(auto it = listChecksum.begin(); it != listChecksum.end(); it++)
				{
					Pair<uint32, uint32> pair = *it;

					//printf("cfc: %d ccs: %d\n", pair.a, pair.b);
						
					//throw error when checksum doesn not match but frame does
					//keep frames that doesnt match
					if( pair.a == serverFramecount)
					{
						if(pair.b == serverChecksum){
							listChecksum.erase(it++);
						}
						else{
							printf("frame: %d, clientChecksum: %d, serverChecksum: %d\n", clientFramecount, pair.b, serverChecksum);
							throw "checksum failed";
						}
					}
				}
			}
			else
			{
				//search entity list for matching entityID
				for(auto itentity = listEntity.begin(); itentity != listEntity.end(); itentity++)
				{
					Entity* entity = *itentity;

					if( message.recpientID == entity->entityID )
					{
						entity->setAFTC(message);
						break;
					}
				}
			}
			
			if(message.messageData != 0)
				delete[] message.messageData;
		}

		//draw game entitys
		{
			//reset drawing buffer
			glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		
			//------------ Camera trixing ------------		
			glLoadIdentity();

			glTranslatef(+gameWidth/40.0f, -gameHeight/40.0f, 0);
			//glTranslatef(-player->GetPosition().x,-player->GetPosition().y,0.0f);
			//printf("x:%+e\ty:%+e\n", player->GetPosition().x, player->GetPosition().y);

			if(playerXXX != 0)
			{
				auto p = playerXXX->body->GetPosition();
				glTranslatef(-p.x, -p.y, +0.0f);
			}

			//draw origo
			DrawPoint( b2Vec2(0,0), 3.0f, b2Color(0.0f,1.0f,0.0f));

			for(auto it = listEntity.begin(); it != listEntity.end(); it++)
			{
				Entity* entity = *it;
				entity->draw();
			}
		}

		window0->swapBuffers(); //will block if v-sync is on

		//todo figure out a better sleep time
		//Thread::Sleep(1000/60);

		//check user interactions
		{
			for(int i=0; i<5; i++)
				window0->run();

	        this->checkControls();
		}
    }
}

void GameClient::checkControls(){
    
    //special keys
    if(keydown[VK_ESCAPE]){
        windowUnfocused();
        ingame = false;
    }
	
	/*
	const float forceConstant = 200.0f * player->GetMass();

	if(keydown[VK_UP])
		player->ApplyForceToCenter(b2Vec2(0.0f,forceConstant));
	if(keydown[VK_DOWN])
		player->ApplyForceToCenter(b2Vec2(0.0f,-forceConstant));
	if(keydown[VK_LEFT])
		player->ApplyForceToCenter(b2Vec2(-forceConstant,0.0f));
	if(keydown[VK_RIGHT])
		player->ApplyForceToCenter(b2Vec2(forceConstant,0.0f));

	if(keydown[VK_SPACE])
		;
	*/

	const float forceConstant = 50.0f;
	
	/*
	if(keydown[VK_UP])
		playerXXX->body->ApplyForceToCenter(b2Vec2(0.0f,forceConstant));
	if(keydown[VK_DOWN])
		playerXXX->body->ApplyForceToCenter(b2Vec2(0.0f,-forceConstant));
	if(keydown[VK_LEFT])
		playerXXX->body->ApplyForceToCenter(b2Vec2(-forceConstant,0.0f));
	if(keydown[VK_RIGHT])
		playerXXX->body->ApplyForceToCenter(b2Vec2(forceConstant,0.0f));
	*/

	if(playerXXX == 0)
		return;

	if(keydown[VK_UP])
	{
		//send move up message
		playerXXX->aftcX += +0.0f;
		playerXXX->aftcY += +forceConstant;
	}

	if(keydown[VK_DOWN])
	{
		playerXXX->aftcX += +0.0f;
		playerXXX->aftcY += -forceConstant;
	}

	if(keydown[VK_LEFT])
	{
		playerXXX->aftcX += -forceConstant;
		playerXXX->aftcY += +0.0f;
	}

	if(keydown[VK_RIGHT])
	{
		playerXXX->aftcX += +forceConstant;
		playerXXX->aftcY += +0.0f;
	}

	if( (playerXXX->aftcX != 0 || playerXXX->aftcY != 0 ) )
	{
		Message message = playerXXX->getAFTC();
		clientXXX->fastSend(message);

		playerXXX->aftcX = 0;
		playerXXX->aftcY = 0;
	}
}
