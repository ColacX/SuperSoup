#pragma comment(lib, "Box2D.lib" )
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

void networkClientTest()
{
	char* targetIP = "127.0.0.1";
	char* targetPort = "12000";
	SOCKET socketClient;

	//start windows socket driver
	WSADATA wsaData;
    
	if(WSAStartup( MAKEWORD(2,2), &wsaData) != 0)
        throw "GameClient: WSAStartup failed";

	//set network to tcp
    struct addrinfo hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	//get all available alias addresses to the targetIP
    struct addrinfo *result = 0;

    if( getaddrinfo(targetIP, targetPort, &hints, &result) != 0 ){
        WSACleanup();
        throw "GameClient: getaddrinfo failed";
    }

	 //attempt to connect to an address until one succeeds
    struct addrinfo *ptr = 0;
    
    for( ptr = result; ptr != 0; ptr = ptr->ai_next )
	{
        socketClient = socket( ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        
        if(socketClient == INVALID_SOCKET)
		{
			//something went terribly wrong
            freeaddrinfo(result);
            WSACleanup();
            throw "GameClient: socket failed";
        }

        if(connect( socketClient, ptr->ai_addr, (int)ptr->ai_addrlen) == SOCKET_ERROR)
		{
			//could not connect
            closesocket(socketClient);
            socketClient = INVALID_SOCKET;
        }
        else
		{
			//success
            break;
        }
    }

	//free memory
    freeaddrinfo(result);

    if(socketClient == INVALID_SOCKET){
        WSACleanup();
		throw "GameClient: socketClient == INVALID_SOCKET failed";
    }

	Client client;
	client.construct( socketClient );

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

	closesocket( socketClient ); //todo determine who should release resources?
	WSACleanup();
}

int main(int argc, char** argv)
{
	try
	{
		bool isNetwork = false;

		for(int i=0; i<argc; i++)
		{
			printf("arg\t%i:\t%s\n", i, argv[i]);

			if( strcmp(argv[i], "/network") == 0 )
				isNetwork = true;
		}

		if( isNetwork )
		{
			networkClientTest();
			getchar();
		}
		else
		{
			GameClient gc;
			gc.run();
		}
	}
	catch( char* ex )
	{
		printf( "%s\n", ex );
		getchar();
	}

	return 1337;
}
