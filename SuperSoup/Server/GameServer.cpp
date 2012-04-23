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
#include "..\shared\Message.hpp"
#include "..\shared\Client.hpp"
#include "..\shared\Entity.h"

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
	bool isQuit;
	unsigned int bufferSize;
	char* bufferConsole;

	void construct()
	{
		isQuit = false;
		bufferSize = 1024;
		bufferConsole = new char[bufferSize];
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

				ZeroMemory(bufferConsole, bufferSize);
				scanf_s("%s", bufferConsole, bufferSize);
				getchar(); //extra needed???

				if(strcmp(bufferConsole, "/quit") == 0)
					isQuit = true;
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
	ground.bodyType = Entity::static_body;
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

	//-----------------------------------------------------------------
	//start window
    Window w0( false, "Server" );
    window0 = &w0;
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
	
	//-----------------------------------------------------------
	std::list<Message> currentListMessages;

	uint32 serverFramecount = 0;
	uint32 serverChecksum = 0;

	std::list<Pair<unsigned int, uint32>> listChecksum;
		
	while( true )
	{
		//check if server should stop running
		if( isQuit )
			break; //stop running

		//check console state
		{
			if( console.isQuit )
				isQuit = true;
		}

		//cleanup inactive clients
		for(auto it = listClient.begin(); it != listClient.end(); )
		{
			Client* client = *it;
			
			if( client->isQuit() )
			{
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
				static bool playbackPlay = false;
				if(playbackPlay)
				{
					newClient->playback();
					continue;
				}

				char* welcomeMessage = "hello new client";
				unsigned int messageSize = strlen(welcomeMessage)+1;

				char* m = new char[messageSize];
				memcpy(m, welcomeMessage, messageSize);

				Message message;
				message.recpientID = 1;
				message.messageSize = messageSize;
				message.messageData = (Message::byte8*)m;

				newClient->fastSend(message);
			}

			//send all current entitys to the new client
			for(auto it = listEntity.begin(); it != listEntity.end(); it++)
			{
				Entity* entity = *it;				
				Message message = entity->getSync(true);
				message.recpientID = 2;
				newClient->fastSend(message);
			}

			//create a player
			Entity* player = new Entity();
			player->positionY = 5;
			player->construct(world);
			listEntity.push_back(player);

			//send player entity to all clients
			for(auto it = listClient.begin(); it != listClient.end(); it++)
			{
				if( *it == newClient )
					continue; //skip this one
				
				Message message = player->getSync();
				message.recpientID = 2;
				Client* client = *it;
				client->fastSend(message);
			}

			//send player specially to new client
			{
				Message message = player->getSync(true);
				message.recpientID = 3;
				newClient->fastSend(message);
			}

			//send framecount specially to new client
			{
				Message message;
				message.recpientID = 4;
				message.messageSize = sizeof(uint32);
				message.messageData = new char[message.messageSize];
			
				unsigned int offset = 0;
				*((uint32*)&message.messageData[offset]) = serverFramecount; offset += sizeof(uint32);

				newClient->fastSend(message);
			}

			printf("accepted client on frame: %d\n", serverFramecount);
		}

		//----------------------------------------------------------------//
		
		//push / fetch / parse incoming messages to memory for this thread
		for(auto it = listClient.begin(); it != listClient.end(); it++)
		{
			Client* client = *it;
			client->pushMessages();
		}

		//create message frames from clients messages
		for(auto it = listClient.begin(); it != listClient.end(); it++)
		{
			Client* client = *it;
		
			while(client->listMessage.size() > 0 )
			{
				Message message = client->listMessage.front();
				client->listMessage.pop_front();

				currentListMessages.push_back(message);
			}
		}

		//push next frame message
		{
			Message message;
			message.recpientID = 0;
			message.messageSize = 0;
			message.messageData = 0;
			currentListMessages.push_back(message);
		}

		//send framecount and checksum
		if(serverFramecount != 0)
		{
			Message message;
			message.recpientID = 5;
			message.messageSize = sizeof(uint32) + sizeof(uint32);
			message.messageData = new char[message.messageSize];
			
			unsigned int offset = 0;
			*((uint32*)&message.messageData[offset]) = serverFramecount; offset += sizeof(uint32);
			*((uint32*)&message.messageData[offset]) = serverChecksum; offset += sizeof(uint32);
			
			currentListMessages.push_back(message);
		}

		//send all messages
		for(auto itm = currentListMessages.begin(); itm != currentListMessages.end(); itm++)
		{
			Message& message = *itm;

			//copy the message and send to all clients
			for(auto ic = listClient.begin(); ic != listClient.end(); ic++)
			{
				Message m = message;
								
				m.messageData = new char[m.messageSize];
				memcpy(m.messageData, message.messageData, m.messageSize);

				Client* client = *ic;
				client->fastSend(m);
			}
		}

		//todo perhaps buffer up some frames for smoother experience?

		//play all messages in frame
		while( currentListMessages.size() > 0)
		{
			Message message = currentListMessages.front();
			currentListMessages.pop_front();

			//handle messages
			if(message.recpientID == 0)
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

				Pair<unsigned int, uint32> p;
				p.a = serverFramecount;
				p.b = serverChecksum;
				//listChecksum.push_back(p);

				//printf("frame: %d\n", serverFramecount);

				//good for debugging
				for(auto itentity = listEntity.begin(); itentity != listEntity.end(); itentity++)
				{
					Entity* entity = *itentity;
					//printf("XXXXX %d\n", serverFramecount);
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
			}
			else if(message.recpientID == 3)
			{
			}
			else if(message.recpientID == 4)
			{
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
		}

		window0->swapBuffers(); //will block if v-sync is on

		//todo figure out a better sleep time
		//Thread::Sleep(1000/60);

		//check user interactions
		{
			for(int i=0; i<5; i++)
				window0->run();
		}
	}

	for( auto it = listClient.begin(); it != listClient.end(); it++ )
	{
		Client& client = **it;
		client.destruct();
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