#pragma comment(lib, "Box2D.lib" )
#pragma comment(lib, "opengl32.lib") //opengl library for 32-bit computers
#pragma comment(lib, "glu32.lib") //openglu32 librarary extension to opengl
#pragma comment(lib, "glew32.lib") //glew librarary extension to opengl
#pragma comment(lib, "Ws2_32.lib" ) //windows sockets v2 32 bit tcp-network library

#include <winsock2.h>
#include <ws2tcpip.h>
#include <Box2D/Box2D.h>
#include <cstdio>

#include "GameClient.h"
using namespace std;

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

	//blocking tcp send
	{
		printf("sending message...\n");

		const char* dataPointer = "hello from client";
		int dataLength = strlen(dataPointer)+1;
		int sendCount = 0;
		
		while( sendCount < dataLength)
		{
			int iResult = send( socketClient, dataPointer, dataLength, sendCount );
			
			if(iResult == SOCKET_ERROR)
				throw "GameClient: send failed";
			else
				sendCount += iResult;
		}
	}

	//blocking tcp receive
	{
		printf("waiting for message...\n");

		unsigned int lengthIncoming = 1000;
		char* dataPointer = new char[lengthIncoming];
		ZeroMemory( dataPointer, lengthIncoming );
		unsigned int receiveCount = 0;

		int recR = recv( socketClient, dataPointer, lengthIncoming, 0);

		if( recR > 0 )
			receiveCount += recR;
		else if( recR == 0 )
			throw "GameClient: recv connection closed";
		else
			throw "GameClient: recv failed";

		printf("message received: %s\n", dataPointer);
		delete[] dataPointer;
	}

	closesocket( socketClient );
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
			for(int i=0; i<10; i++)
				networkClientTest();
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
