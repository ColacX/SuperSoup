#include "GameServer.hpp"

#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <list>

#include "..\shared\Thread.hpp"
#include "..\shared\Semaphore.hpp"
#include "..\shared\CircularBuffer.hpp"
#include "..\shared\Pair.hpp"
#include "..\shared\\Message.hpp"

typedef Pair<unsigned int, char*> DATAPAIR;

//todo give better name?
class Sender : public Runnable
{
private:
	SOCKET socket;
	CircularBuffer<DATAPAIR> circularBuffer;
	Semaphore semaphore;

public:
	bool isQuit;

	void construct(SOCKET socket)
	{
		this->socket = socket;
		isQuit = false;

		unsigned int bufferCapacity = 1024;
		circularBuffer.construct(bufferCapacity);
		semaphore.construct(0, bufferCapacity);
	}

	void destruct()
	{
		circularBuffer.destruct();
		semaphore.destruct();
	}

	void run()
	{
		try
		{
			while(true)
			{
				if( isQuit )
					break;
			
				//wait for buffer to be filled
				semaphore.wait();

				//fetch data from ram memory
				if(circularBuffer.isEmpty())
					throw "Sender: buffer isEmpty";

				DATAPAIR datapair = circularBuffer.popItem();
				int dataLength = datapair.first;
				char* dataPointer = datapair.second;
				int transmitCount = 0;
			
				//transmit all bytes through network
				while( transmitCount < dataLength)
				{
					int sendR = send( socket, dataPointer, dataLength, transmitCount );
						
					if(sendR == SOCKET_ERROR)
						throw "Sender: send failed";
					else
						transmitCount += sendR;
				}

				//free memory when done
				delete[] dataPointer; //will fail if you send non-deletable data
			}
		}
		catch(char* ex)
		{
			//todo handle errors / disconnects
			printf("%s\n", ex);
		}
	}

	bool isFull()
	{
		return circularBuffer.isFull();
	}

	//remember to only send deletable data
	void addItem( DATAPAIR datapair )
	{
		if(isFull())
			throw "GameServer: buffer isFull";

		circularBuffer.addItem(datapair);

		//signal buffer has item
		semaphore.post();
	}
};

//todo give better name?
class Receiver : public Runnable
{
private:
	SOCKET socket;
	CircularBuffer<DATAPAIR> circularBuffer;
	Semaphore semaphore;
	unsigned int bufferSize;
	char* bufferReceive;

public:
	bool isQuit;

	void construct(SOCKET socket)
	{
		this->socket = socket;

		isQuit = false;

		unsigned int bufferCapacity = 1024;
		circularBuffer.construct(bufferCapacity);
		semaphore.construct(bufferCapacity, bufferCapacity);

		bufferSize = 2048;
		bufferReceive = new char[bufferSize];
	}

	void destruct()
	{
		delete[] bufferReceive;
		circularBuffer.destruct();
	}

	void run()
	{
		try
		{
			while(true)
			{
				if(isQuit)
					break;

				//wait for free buffer space
				semaphore.wait();
			
				if( circularBuffer.isFull() )
					throw "Receiver: circularBuffer.isFull()";

				//fetch data from network into local buffer
				int retrieveCount = 0;
				int recvR = recv( socket, bufferReceive, bufferSize, 0);

				if( recvR > 0 )
					retrieveCount += recvR; //add receviced bytes
				else if( recvR == 0 )
					throw "Receiver: recv connection closed";
				else
					throw "Receiver: recv failed";

				//todo add check if list is full

				//push received network data to ram memory
				char* dataPointer = new char[retrieveCount];
				memcpy( dataPointer, bufferReceive, retrieveCount );
				circularBuffer.addItem( DATAPAIR(retrieveCount, dataPointer) );
			}
		}
		catch(char* ex)
		{
			//todo handle errors / disconnects
			printf("%s\n", ex);
		}
	}

	bool isEmpty()
	{
		return circularBuffer.isEmpty();
	}

	DATAPAIR popItem()
	{
		if( isEmpty() )
			throw "Receiver: buffer isEmpty";
		
		DATAPAIR datapair = circularBuffer.popItem();

		//signal buffer has free slot
		semaphore.post();
		
		return datapair;
	}
};

//todo give better name?
class Client
{
private:
	SOCKET socket;
	Thread receiverThread;
	Thread senderThread;

public:
	Receiver receiver;
	Sender sender;

	void construct( SOCKET socket )
	{
		this->socket = socket;

		//network input
		receiver.construct(socket);
		receiverThread.construct(receiver);
		receiverThread.start();

		//network output
		sender.construct(socket);
		senderThread.construct(sender);
		senderThread.start();
	}

	void destruct()
	{
		//free socket
		closesocket(socket);
		
		//wait for threads to stop
		receiver.isQuit = true;
		sender.isQuit = true;

		receiverThread.destruct();
		senderThread.destruct();

		//free memory
		receiver.destruct();
		sender.destruct();
	}
};

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
				DATAPAIR datapair;
				char* welcomeMessage = "hello new client";

				char* m = new char[100];
				memcpy(m, welcomeMessage, 100);

				datapair.first = 100;
				datapair.second = m;

				client->sender.addItem( datapair );
			}

			//receive message from client
			{
				while( client->receiver.isEmpty() )
					Thread::Sleep(100);

				DATAPAIR datapair = client->receiver.popItem();
				printf("message from client: %s\n", datapair.second);
				delete[] datapair.second;
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