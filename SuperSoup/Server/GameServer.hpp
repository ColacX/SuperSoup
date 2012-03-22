#pragma once

#include <winsock2.h>
#include <stdio.h>
#include <windows.h>

class GameServer
{
private:

public:
	void run()
	{
		//show user options
			//create world
			//choose world
			//choose listenPort
		//load world
		//start drivers
		
		//loop start
			//check if server should stop running
				//stop running

			//only do this once every 5s to save some performance
				//peek accept new clients
				//accept new clients if any

			//simulate world
			//push_back all needsync objects to clients
			//pop and send all messages to clients if any
			//peek for messages from clients
			//fetch all message if any
			//handle messages from clients
				//? reserved space
		//loop end

		char* listenPort;
		listenPort = "12000"; //does nothing atm
		
		//start window socket driver
		WSADATA wsaData;
		
		if(WSAStartup(MAKEWORD(2,2), &wsaData) != NO_ERROR)
			throw "GameServer: WSAStartup failed";

		//create listen socket
		SOCKET listenSocket;
		listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		if(listenSocket == INVALID_SOCKET){
			WSACleanup();
			throw "GameServer: socket failed";
		}

		//bind listenSocket to the listenPort
		sockaddr_in service;
		service.sin_family = AF_INET;
		service.sin_addr.s_addr = inet_addr("127.0.0.1");
		service.sin_port = htons(12000); //todo use listenPort

		if( bind( listenSocket, (SOCKADDR*)&service, sizeof(service)) == SOCKET_ERROR ){
			closesocket(listenSocket);
			WSACleanup();			
			throw "GameServer: bind failed";
		}

		if( listen(listenSocket, 1) == SOCKET_ERROR ){
			closesocket(listenSocket);
			WSACleanup();
			throw "GameServer: listen failed";
		}

		bool isRunning;
		isRunning = true;
		printf("tcp server started listening on port: %s\n", listenPort);

		while( isRunning )
		{
			SOCKET* socketClient = new SOCKET;
			
			struct sockaddr_in newAddress;
			int lengthAdress = sizeof(sockaddr_in);

			*socketClient = accept( listenSocket, (sockaddr*)&newAddress, &lengthAdress );
        
			if(*socketClient == INVALID_SOCKET)
			{
				printf("accept failed: %d\n", WSAGetLastError());
				isRunning = false;
				closesocket(listenSocket);
				WSACleanup();
				break;
			}
			else
			{
				printf("new client connected from: %d.%d.%d.%d\n",
					newAddress.sin_addr.S_un.S_un_b.s_b1,
					newAddress.sin_addr.S_un.S_un_b.s_b2,
					newAddress.sin_addr.S_un.S_un_b.s_b3,
					newAddress.sin_addr.S_un.S_un_b.s_b4);

				//tcp blocking send
				{
					printf("sending message...\n");

					char* dataPointer = "hello from server";
					int dataLength = strlen(dataPointer);
					int transmitCount = 0;
					
					while( transmitCount < dataLength)
					{
						int sendR = send( *socketClient, dataPointer, dataLength, transmitCount );
						
						if(sendR == SOCKET_ERROR)
							throw "GameServer: send failed";
						else
							transmitCount += sendR;
					}
				}

				//tcp blocking recive
				{
					printf("waiting for message...\n");

					unsigned int lengthIncoming = 1000;
					char* dataPointer = new char[lengthIncoming];
					ZeroMemory( dataPointer, lengthIncoming );
					int retrieveCount = 0;

					int recvR = recv( *socketClient, dataPointer, lengthIncoming, 0);

					if( recvR > 0 )
						retrieveCount += recvR;
					else if( recvR == 0 )
						throw "GameServer: recv connection closed";
					else
						throw "GameServer: recv failed";

					printf("%s\n", dataPointer);
					delete[] dataPointer;
				}
			}

			delete socketClient;
		}
	}
};