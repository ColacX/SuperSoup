#include "GameServer.hpp"

#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <list>

#include "..\shared\Thread.hpp"
#include "..\shared\Semaphore.hpp"
#include "..\shared\Pair.hpp"
#include "..\shared\Message.hpp"
#include "..\shared\Client.hpp"

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
			throw "Accepter: WSAStartup failed";

		//create listen socket
		listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		if(listenSocket == INVALID_SOCKET)
			throw "Accepter: socket failed";

		//bind listenSocket to the listenPort
		int port = atoi(listenPort);

		if(0 >= port || port >= 65535)
			throw "Accepter: listenPort is out of range";

		sockaddr_in socketArgs;
		socketArgs.sin_family = AF_INET;
		socketArgs.sin_addr.s_addr = inet_addr("127.0.0.1");
		socketArgs.sin_port = htons(port);

		if( bind( listenSocket, (SOCKADDR*)&socketArgs, sizeof(socketArgs)) == SOCKET_ERROR )
			throw "Accepter: bind failed";

		if( listen(listenSocket, 1) == SOCKET_ERROR )
			throw "Accepter: listen failed";
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
				playerCountSemaphore.wait();
			
				if(playerCount >= playerMax)
					throw "playerCount >= playerMax";

				//accept new clients if any
				struct sockaddr_in newAddress;
				int lengthAdress = sizeof(sockaddr_in);
				SOCKET socketClient;

				socketClient = accept( listenSocket, (sockaddr*)&newAddress, &lengthAdress );

				if(socketClient == INVALID_SOCKET)
					throw "Accepter: accept failed";
			
				printf("Accepter: new connection accepted from: %d.%d.%d.%d\n",
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

	std::list<Client*> clientList;
	
	//start accepting clients
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

		//peek for new client
		if( accepter.isFull )
		{
			//fetch new client
			Client* client = accepter.FetchClient();

			//welcome new client

			//send welcome message
			{
				char* welcomeMessage = "hello new client";

				char* m = new char[100];
				memcpy(m, welcomeMessage, 100);

				Message message;
				message.recpientID = 10;
				message.messageSize = 100;
				message.messageData = (Message::byte8*)m;

				client->fastSend( message );
			}

			//receive message from client
			{
				while( client->receiver.isEmpty() )
					Thread::Sleep(100);

				Pair<unsigned int, char*> datapair = client->receiver.popItem();
				printf("message from client: %s\n", datapair.b);
				delete[] datapair.b;
			}

			//store new client
			clientList.push_back(client);
		}

		//simulate world
		//push_back all needsync objects to clients
		//pop and send all messages to clients if any
		//peek for messages from clients
		

		//fetch all message if any
		//handle messages from clients
			//? reserved space

		Thread::Sleep(1000/60);
	}

	for( auto it = clientList.begin(); it != clientList.end(); it++ )
	{
		Client* client = *it;
		client ->destruct();
	}

	accepter.destruct();
	accepterThread.destruct();

	console.destruct();
	consoleThread.destruct();
}