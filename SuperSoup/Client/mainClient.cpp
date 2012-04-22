#pragma comment(lib, "Box2D.lib" ) //box2d physics engine library
#pragma comment(lib, "opengl32.lib") //opengl library for 32-bit computers
#pragma comment(lib, "glu32.lib") //openglu32 librarary extension to opengl
#pragma comment(lib, "glew32.lib") //glew librarary extension to opengl
#pragma comment(lib, "Ws2_32.lib" ) //windows sockets v2 32 bit tcp-network library

#include <winsock2.h>
#include <ws2tcpip.h>
#include <Box2D/Box2D.h>
#include <cstdio>
#include <list>

#include "GameClient.h"

#include "..\shared\Message.hpp"
#include "..\shared\Receiver.hpp"
#include "..\shared\Thread.hpp"
#include "..\shared\Pair.hpp"
#include "..\shared\Client.hpp"

#include "..\shared\SharedMisc.hpp"

void networkClientTest()
{
	Client client;
	client.construct( Client::connectTo("127.0.0.1", "12000") );

	{
		printf("sending message...\n");

		char* messageData = "hello from client";
		unsigned int messageSize = strlen(messageData)+1;

		char* m = new char[messageSize];
		memcpy(m, messageData, messageSize);

		Message message;
		message.recpientID = 10;
		message.messageSize = messageSize;
		message.messageData = (Message::byte8*)m;

		client.fastSend( message );
	}

	printf("waiting for message...\n");

	while(true)
	{
		Thread::Sleep(1000/60);
		client.pushMessages();
		
		if(client.listMessage.size() > 0 )
		{
			Message newMessage = client.listMessage.front();
			client.listMessage.pop_front();
			printf("%s\n", newMessage.messageData);
			delete[] newMessage.messageData;
		}
	}
}


int main(int argc, char** argv)
{
	MemoryDebug::isEnabled = true;
	
	try
	{
		bool isNetwork = false;

		for(int i=0; i<argc; i++)
		{
			printf("arg\t%i:\t%s\n", i, argv[i]);

			if( strcmp(argv[i], "/network") == 0 )
				isNetwork = true;
		}

		GameClient gc;

		if( isNetwork )
		{
			//networkClientTest();
			gc.run2();
		}
		else
		{
			gc.run();
		}
	}
	catch( char* ex )
	{
		printf( "%s\n", ex );
		getchar();
	}
	
	memoryDebug.debugPrint();
	MemoryDebug::isEnabled = false;
	return 1337;
}
