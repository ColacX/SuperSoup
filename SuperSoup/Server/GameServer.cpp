#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <list>
#include <Box2D\Box2D.h>
#include <gl\GLU.h>
#include <gl\GL.h>

#include "..\shared\Thread.hpp"
#include "..\shared\Semaphore.hpp"
#include "..\shared\Pair.hpp"
#include "..\shared\Client.hpp"
#include "..\shared\Entity.h"
#include "..\shared\MessageSystem.h"

#include "GameServer.hpp"

#include "..\shared\SharedMisc.hpp"

//todo give better name?
class Accepter : public Runnable
{
private:
	Semaphore playerCountSemaphore; //todo post when players releases slot	
	Semaphore clientSemaphore; //todo replace with mutex
	Client* client;
	unsigned int playerCount;
	unsigned int playerMax;
	char* listenPort;
	SOCKET listenSocket;

public:
	bool isQuit;
	bool isFull;

	void construct()
	{
		isFull = false;
		isQuit = false;
		playerCount = 0;
		playerMax = 8;
		listenPort = "12000";

		clientSemaphore.construct(1,1); //todo replace with mutex
		playerCountSemaphore.construct( playerMax, playerMax );

		//start window socket driver
		WSADATA wsaData;
		
		if(WSAStartup(MAKEWORD(2,2), &wsaData) != NO_ERROR)
			throw "WSAStartup failed";

		//create listen socket
		listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		if(listenSocket == INVALID_SOCKET)
			throw "socket failed";

		//bind listenSocket to the listenPort
		int port = atoi(listenPort);

		if(0 >= port || port >= 65535)
			throw "listenPort is out of range";

		sockaddr_in socketArgs;
		socketArgs.sin_family = AF_INET;
		socketArgs.sin_addr.s_addr = inet_addr("127.0.0.1");
		socketArgs.sin_port = htons(port);

		if( bind( listenSocket, (SOCKADDR*)&socketArgs, sizeof(socketArgs)) == SOCKET_ERROR )
			throw "bind failed";

		if( listen(listenSocket, 1) == SOCKET_ERROR )
			throw "listen failed";
	}

	void destruct()
	{
		isQuit = true;
		closesocket(listenSocket);
		WSACleanup();

		playerCountSemaphore.destruct();
		clientSemaphore.destruct();
	}

	void run()
	{
		printf("tcp server started listening on port: %s\n", listenPort);

		try{
			while(true)
			{
				if(isQuit)
					break;

				//wait for server to fetch new client
				clientSemaphore.wait();

				if(isFull)
					throw "isFull";

				//wait for free player slot
				//playerCountSemaphore.wait();
			
				//if(playerCount >= playerMax)
					//throw "playerCount >= playerMax";

				//accept new clients if any
				struct sockaddr_in newAddress;
				int lengthAdress = sizeof(sockaddr_in);
				SOCKET socketClient;

				socketClient = accept( listenSocket, (sockaddr*)&newAddress, &lengthAdress );

				if(socketClient == INVALID_SOCKET)
					throw "accept failed";
			
				printf("new connection accepted from: %d.%d.%d.%d\n",
					newAddress.sin_addr.S_un.S_un_b.s_b1,
					newAddress.sin_addr.S_un.S_un_b.s_b2,
					newAddress.sin_addr.S_un.S_un_b.s_b3,
					newAddress.sin_addr.S_un.S_un_b.s_b4);

				client = new Client;

				//create threads for handling i/o to client
				client->construct(socketClient);
						
				isFull = true;			
			}
		}
		catch(char* ex)
		{
			printf("%s\n", ex);
		}
	}

	Client* FetchClient()
	{
		Client* client = this->client;

		//signal that the client has been taken
		isFull = false;
		clientSemaphore.post();

		return client;
	}
};


//todo give better name?
class Console : public Runnable
{
private:

public:
	unsigned int bufferSize;
	char* bufferConsole;
	bool isQuit;
	bool isStart;
	bool isGraphic;

	void construct()
	{
		bufferSize = 1024;
		bufferConsole = new char[bufferSize];

		isQuit = false;
		isGraphic = false;
		isStart = false;
	}

	void destruct()
	{
		delete[] bufferConsole;
	}

	void run()
	{
		try
		{
			while(true)
			{
				if(isQuit)
					break; //quit

				if(isStart)
					Thread::Sleep(1000); //waiting to start game

				ZeroMemory(bufferConsole, bufferSize);
				scanf_s("%s", bufferConsole, bufferSize);
				getchar(); //extra needed???

				if(strcmp(bufferConsole, "/help") == 0)
				{
					printf("available commands:\n");
					printf("/help\n");
					printf("/quit\n");
					printf("/graphic\n");
				}
				else if(strcmp(bufferConsole, "/quit") == 0)
					isQuit = true;
				else if(strcmp(bufferConsole, "/graphic") == 0)
					isGraphic = true;
				else if(strcmp(bufferConsole, "/start") == 0)
					isStart = true;
				else
					printf("Console: unknown command\n");
			}
		}
		catch(char* ex)
		{
			printf("%s\n", ex);
		}
	}
};

void GameServer::run()
{
	bool isQuit;
	isQuit = false;

	window0 = 0;

	//show user options
		//create world
		//choose world
		//choose listenPort
	
	//load world

	std::list<Entity*> listEntity;

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

	//ground
	Entity ground;
	ground.shapeWidth = 100;
	ground.shapeHeight = 1;
	ground.bodyType = b2_staticBody;
	ground.construct(world);
	listEntity.push_back(&ground);

	//ball0
	Entity ball0;
	ball0.positionY = 10;
	ball0.shapeWidth = 2;
	ball0.construct(world);
	listEntity.push_back(&ball0);

	//----------------------------------------------------------------------------------

	std::list<Client*> listClient;
	
	Accepter accepter;
	accepter.construct();

	Thread accepterThread;
	accepterThread.construct(accepter);
	accepterThread.start();

	Console console;
	console.construct();

	Thread consoleThread;
	consoleThread.construct(console);
	consoleThread.start();

	//-----------------------------------------------------------
	
	uint32 serverFramecount = 0;
	uint32 serverChecksum = 0;
	std::list<M*> messageFrame;
		
	while( true )
	{
		//todo figure out a better sleep time
		Thread::Sleep(1000/60);

		//check if server should stop running
		if( isQuit )
			break; //stop running

		//check some console stuff
		{
			if( console.isQuit )
				isQuit = true;

			if( console.isGraphic && window0 == 0)
			{
				//start window
				window0 = new Window( false, "Server" );
				//window0->setSize( 1920, 1080 );
				window0->addWindowListener( this );
				window0->create();
				//window0->setMaximized();

				//get device context and rendering context
				HDC windowDeviceContext0 = window0->getDeviceContext();
				HGLRC renderingContext0 = wglCreateContext( windowDeviceContext0 );

				//this thread must own the gl rendering context or gl calls will be ignored
				BOOL rwglMakeCurrent = wglMakeCurrent( windowDeviceContext0, renderingContext0);
    
				if(!rwglMakeCurrent)
					throw "wglMakeCurrent failed";

				glHint(GL_LINE_SMOOTH, GL_NICEST);
				glEnable(GL_LINE_SMOOTH);

				windowResized(window0->getWidth(), window0->getHeight() );
			}
		}

		//cleanup inactive clients
		for(auto it = listClient.begin(); it != listClient.end(); )
		{
			Client* client = *it;
			
			if( client->isQuit() )
			{
				printf("deleting client...\n");
				client->destruct();
				listClient.erase(it++);
			}
			else{
				it++;
			}
		}

		//peek for new client
		if( accepter.isFull )
		{
			printf("client accepting process begin\n");
			printf("begin accepting new client on frame: %d\n", serverFramecount);

			//fetch new client
			Client* newClient = accepter.FetchClient();
			//store new client
			listClient.push_back(newClient);

			//welcome new client

			//send welcome message
			{
				newClient->send( new M_TextAll("hello new client") );
			}

			//send all current entitys to the new client
			for(auto it = listEntity.begin(); it != listEntity.end(); it++)
			{
				Entity* entity = *it;				
				M* m = entity->getCreate();
				newClient->send(m);
			}

			//create a player
			Entity* player = new Entity();
			player->positionY = 5;
			player->construct(world);
			listEntity.push_back(player);

			//send player entity to all clients
			for(auto it = listClient.begin(); it != listClient.end(); it++)
			{
				Client* client = *it;
				M* m = player->getCreate();
				client->send(m);
			}

			//assign player entity to the new client
			{
				M* m = new M_EntityAssign(player->entityID);
				newClient->send(m);
			}

			//send game framecount to new client
			{
				M* m = new M_GameFrame(serverFramecount);
				newClient->send(m);
			}

			printf("accepted client on frame: %d\n", serverFramecount);
		}

		if(!console.isStart)
		{
			continue;
		}

		//----------------------------------------------------------------//
		
		//push / fetch / parse incoming messages to memory for this thread
		for(auto it = listClient.begin(); it != listClient.end(); it++)
		{
			Client* client = *it;
			client->run();
		}

		//handle messages from clients and put as many messages possible into a frame of messages
		for(auto it = listClient.begin(); it != listClient.end(); it++)
		{
			Client* client = *it;
		
			while(true)
			{
				if(client->listM.size() == 0 )
					break;

				M* m = client->listM.front();
				client->listM.pop_front();

				if(m->id == M::E_GameResync)
				{
					//client requested a game resync
					printf("client requested a game resync\n");

					//re synchronize all entitys with client
					for(auto it = listEntity.begin(); it != listEntity.end(); it++)
					{
						Entity* entity = *it;				
						M_EntitySync* entitySync = entity->getSync();
						client->send(entitySync);

						printf("sending entity sync message size\n");
					}

					delete m;
				}
				else
				{
					messageFrame.push_back(m);
				}
			}
		}

		//push game next step message into the message frame
		{
			M* m = new M_GameStep;
			messageFrame.push_back(m);
		}

		//push framecount and checksum into the message frame
		if(serverFramecount != 0)
		{
			M* m = new M_GameChecksum( serverFramecount, serverChecksum );
			messageFrame.push_back(m);
		}

		//send all messages from message frame
		for(auto itm = messageFrame.begin(); itm != messageFrame.end(); itm++)
		{
			M* m = *itm;

			//copy the message and send to all clients
			for(auto itc = listClient.begin(); itc != listClient.end(); itc++)
			{
				Client* client = *itc;
				client->send( m->clone() );
			}

			//dont delete m;
		}

		//todo perhaps buffer up some frames for smoother experience?

		//play all messages in message frame
		while( true )
		{
			if(messageFrame.size() == 0 )
				break;

			M* m = messageFrame.front();
			messageFrame.pop_front();

			//handle messages
			if(m->id == M::E_EntityAssign)
			{
			}
			else if(m->id == M::E_EntityCreate)
			{
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
			}
			else if(m->id == M::E_GameChecksum)
			{
			}
			else if(m->id == M::E_GameFrame)
			{
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

				serverFramecount++;
				serverChecksum = serverFramecount;

				for(auto it = listEntity.begin(); it != listEntity.end(); it++)
				{
					Entity* entity = *it;
					serverChecksum += entity->getChecksum();
				}

				//printf("frame: %d\n", serverFramecount);
			}
			else if(m->id == M::E_TextAll)
			{
				//print a message
				printf("text all message: %s\n", ((M_TextAll*)m)->text);
			}
			else
			{
				throw "undefined message id";
			}

			delete m;
		}

		if(console.isGraphic)
		{
			//draw game entitys
			//reset drawing buffer
			glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		
			//------------ Camera trixing ------------		
			glLoadIdentity();

			int gameWidth = window0->getWidth();
			int gameHeight = window0->getHeight();

			glTranslatef(+gameWidth/40.0f, -gameHeight/40.0f, 0);
			//glTranslatef(-player->GetPosition().x,-player->GetPosition().y,0.0f);
			//printf("x:%+e\ty:%+e\n", player->GetPosition().x, player->GetPosition().y);

			for(auto it = listEntity.begin(); it != listEntity.end(); it++)
			{
				Entity* entity = *it;
				entity->draw();
			}

			window0->swapBuffers(); //will block if v-sync is on

			//check user interactions
			{
				for(int i=0; i<5; i++)
					window0->run();
			}
		}
	}

	for( auto it = listClient.begin(); it != listClient.end();  )
	{
		Client* client = *it;
		client->destruct();
		delete client;

		listClient.erase(it++);
	}

	accepter.destruct();
	accepterThread.destruct();

	console.destruct();
	consoleThread.destruct();
}

void GameServer::windowResized(int width, int height)
{
	glViewport(0, 0, width, height);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, width/20.0f,-height/20.0f, 0);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	
	glTranslatef(+width/40.0f, -height/40.0f, 0);
}