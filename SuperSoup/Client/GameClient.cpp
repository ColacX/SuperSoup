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

GameClient::GameClient()
{
    isRunning = false;
    clientFramecount = 0;
    ZeroMemory( keydown, sizeof(keydown) );
    window0 = 0;
    ingame = false;
	playerXXX = 0;
	clientXXX = 0;
	targetIP = "127.0.0.1";
	targetPort = "12000";

	lastPositionX = 0;
	lastPositionY = 0;
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
	mx = localX;
	my = localY;

	/*
	float k = 20.0f;
	float mxInWorld = player->GetPosition().x + (mx-int(window0->getWidth()) /2)/k;
	float myInWorld = player->GetPosition().y - (my-int(window0->getHeight())/2)/k;

	
	if(button == MouseListener::BUTTON_LEFT)
		ground->add(round(mxInWorld),round(myInWorld));
	else if(button == MouseListener::BUTTON_RIGHT)
		ground->del(round(mxInWorld),round(myInWorld));
	*/

	lastPositionX = localX;
	lastPositionY = localY;
}

void GameClient::mouseReleased(unsigned int button, int localX, int localY){
	/*
	player->ApplyForceToCenter(100*b2Vec2( - (lastPositionX - localX), lastPositionY - localY ));
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

	ground = new Ground(world);

	// Define the ground body.
	Object groundObj;
	groundObj.createBox(world,b2Vec2(0,-1),b2Vec2(100,1),0,b2_staticBody);
	ground->objects.push_back(groundObj);
	
	/*
	size_t antal = 0;
	for(float y=10.0f; y<=30.0f/2; y+=1.0f){
		for(float x=-20.0f/2; x<=20.0f/2; x+=1.0f){
			Object other;
			other.createBox(world,b2Vec2(x,y));
			objects.push_back(other);
			++antal;
		}
	}
	printf( "\nantal boxar som skapas = %u\n\n", antal );
	*/
	//create new entity and add it to the game

	std::vector<Entity*> listTower;

	for(int i=0; i<10; i++)
	{
		Entity* entity = new Entity();

		entity->positionX = 2;
		entity->positionY = 2+i;
		entity->construct(world);
		listTower.push_back(entity);

		b2RevoluteJointDef jointDef;

		if(i==0)
		{
			jointDef.bodyA = groundObj.body;
			jointDef.bodyB = entity->body;
			jointDef.localAnchorA.Set(0, 1.0f);// = jointDef.bodyA->GetPosition();
			jointDef.localAnchorB.Set(0, -0.5f);
		}
		else
		{
			jointDef.bodyA = listTower[i]->body;
			jointDef.bodyB = listTower[i-1]->body;
			jointDef.localAnchorA.Set(0, -0.5f);// = jointDef.bodyA->GetPosition();
			jointDef.localAnchorB.Set(0, 0.5f);
		}

		b2RevoluteJoint* joint = (b2RevoluteJoint*)world.CreateJoint(&jointDef);
		joint->EnableLimit(true);
		
		//world->DestroyJoint(joint);
		//joint = NULL;
	}

	
	
	//bool b = g.isBlock(b2Vec2(1.0f,2.0f));
	
	Object playerObj;
	playerObj.createBox(world,b2Vec2(0,2));
	playerObj.body->SetBullet(true);
	ground->objects.push_back(playerObj);

	player = playerObj.body;
	b2MassData md;
	md.center = b2Vec2_zero;
	md.I = 2.6666667f;	// 8/3 is the default value.
	md.mass = 40.0f;
	player->SetMassData(&md);

	b2RevoluteJointDef jointDef;
	jointDef.bodyA = player;
	jointDef.bodyB = listTower.back()->body;
	jointDef.localAnchorA.Set(0, +1.0f);// = jointDef.bodyA->GetPosition();
	jointDef.localAnchorB.Set(0, -0.5f);
	b2RevoluteJoint* joint = (b2RevoluteJoint*)world.CreateJoint(&jointDef);
	
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

		std::for_each(listTower.begin(), listTower.end(), [] (Entity* ptr)
		{
			ptr->draw();
		});

		for(size_t o=0; o<ground->objects.size(); ++o)
			ground->objects[o].DebugDrawBox();

		// How to get and draw mouse on the screen
		//float k = 20.0f;
		//float mxInWorld = player->GetPosition().x + (mx-int(window0->getWidth()) /2)/k;
		//float myInWorld = player->GetPosition().y - (my-int(window0->getHeight())/2)/k;
		//Ground::drawCube(mxInWorld,myInWorld);
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
		//construct network messages and put them into the list
		client.run();
		
		while( true )
		{
			//stop when theres no message left in the list
			if(client.listM.size() == 0)
				break;

			//fetch message
			M* m = client.listM.front();
			client.listM.pop_front();
			
			if(m->id == M::E_EntityAssign)
			{
				//find matching entity id
				for(auto it = listEntity.begin(); it != listEntity.end(); it++)
				{
					Entity* entity = *it;

					if( ((M_EntityAssign*)m)->entityID == entity->entityID )
					{
						//set as player
						playerXXX = entity;
						entity->getSync();
						break;
					}
				}
			}
			else if(m->id == M::E_EntityCreate)
			{
				//create new entity and add it to the game
				Entity* entity = new Entity();
				entity->setSync(*(M_EntitySync*)m);
				entity->construct(world);
				listEntity.push_back(entity);
			}
			else if(m->id == M::E_EntityForce)
			{
				//search entity list for matching entityID
				for(auto it = listEntity.begin(); it != listEntity.end(); it++)
				{
					Entity* entity = *it;

					if( ((M_EntityForce*)m)->entityID == entity->entityID )
					{
						entity->setAFTC(*(M_EntityForce*)m);
						break;
					}
				}
			}
			else if(m->id == M::E_EntitySync)
			{
				//todo add check if entity is not in list then something is really offsync
				//search entity list for matching entityID
				for(auto it = listEntity.begin(); it != listEntity.end(); it++)
				{
					Entity* entity = *it;

					if( ((M_EntitySync*)m)->entityID == entity->entityID )
					{
						printf("synced entityID: %d\n", ((M_EntitySync*)m)->entityID);
						entity->reSync(*(M_EntitySync*)m);
						break;
					}
				}
			}
			else if(m->id == M::E_GameChecksum)
			{
				uint32 serverFramecount = ((M_GameChecksum*)m)->framecount;
				uint32 serverChecksum = ((M_GameChecksum*)m)->checksum;

				for(auto it = listChecksum.begin(); it != listChecksum.end(); it++)
				{
					Pair<uint32, uint32> pair = *it;
					
					//throw error when checksum doesn not match but frame does
					//keep frames that doesnt match
					if( pair.a == serverFramecount)
					{
						if(pair.b == serverChecksum)
						{
							listChecksum.erase(it++);
						}
						else
						{
							printf("checksum failed: frame: %d, clientChecksum: %d, serverChecksum: %d\n", clientFramecount, pair.b, serverChecksum);

							if( clientFramecount % 120 == 0)
							{
								//request a game full re sync from server
								clientXXX->send(new M_GameResync);
							}
						}
					}
				}
			}
			else if(m->id == M::E_GameFrame)
			{
				uint32 serverFramecount = ((M_GameFrame*)m)->framecount;
				clientFramecount = serverFramecount;
				printf("clientFramecount set from server: %d\n", clientFramecount);
			}
			else if(m->id == M::E_GameStep)
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
				//printf("frame: %d\n", clientFramecount);

				for(auto it = listEntity.begin(); it != listEntity.end(); it++)
				{
					Entity* entity = *it;
					//printf("XXXXX %d\n", clientFramecount);
					entity->getSync();
				}
			}
			else if(m->id == M::E_TextAll)
			{
				printf("text all message: %s\n", ((M_TextAll*)m)->text);
			}
			else
			{
				throw "undefined message id";
			}

			delete m; //free message memory
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
    if(keydown[VK_ESCAPE])
	{
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

	/*
	const float forceConstant = 50.0f;
	
	if(keydown[VK_UP])
		playerXXX->body->ApplyForceToCenter(b2Vec2(0.0f,forceConstant));
	if(keydown[VK_DOWN])
		playerXXX->body->ApplyForceToCenter(b2Vec2(0.0f,-forceConstant));
	if(keydown[VK_LEFT])
		playerXXX->body->ApplyForceToCenter(b2Vec2(-forceConstant,0.0f));
	if(keydown[VK_RIGHT])
		playerXXX->body->ApplyForceToCenter(b2Vec2(forceConstant,0.0f));
	*/

	const float forceConstant = 1.0f;

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
		M* m = playerXXX->getAFTC(); //todo improve this
		clientXXX->send(m);

		playerXXX->aftcX = 0;
		playerXXX->aftcY = 0;
	}
}
