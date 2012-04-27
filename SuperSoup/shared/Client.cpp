#include <list>
#include <ws2tcpip.h>
#include <winsock2.h>

#include "Client.hpp"

#include "SharedMisc.hpp"

SOCKET Client::connectTo(const char* targetIP, const char* targetPort)
{
	SOCKET socketClient;

	//start windows socket driver
	WSADATA wsaData;
    
	if(WSAStartup( MAKEWORD(2,2), &wsaData) != 0)
        throw "WSAStartup failed";

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
        throw "getaddrinfo failed";
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
            throw "socket failed";
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
		throw "socketClient == INVALID_SOCKET failed";
    }

	return socketClient;
}

void Client::construct( SOCKET socket )
{
	this->connectionSocket = socket;

	//network input
	receiver.construct(socket);
	receiverThread.construct(receiver);
	receiverThread.start();

	//network output
	sender.construct(socket);
	senderThread.construct(sender);
	senderThread.start();

	bufferMain.a = 0;
	bufferMain.b = 0;
}

void Client::destruct()
{
	//interrupt the thread locks
	closesocket(connectionSocket);
		
	receiver.close();
	sender.close();
	
	//wait for threads to stop
	receiverThread.destruct();
	senderThread.destruct();

	//free memory
	receiver.destruct();
	sender.destruct();
}

void Client::send(const M* m)
{
	//printf("sending M->id: %d\n", m->id);

	Pair<unsigned int, char*> datapair;
	datapair.a = M::struct_size[m->id];
	datapair.b = (char*)m;
	sender.addItem(datapair);
}

void Client::run()
{
	while( true )
	{
		//stop if receiver is empty
		if(receiver.isEmpty())
			break;

		//fetch new data
		Pair<unsigned int, char*> dataPacket;
		dataPacket = receiver.popItem(); 

		//create a new buffer that can contain the old data and new data //todo improve this?
		Pair<unsigned int, char*> bufferTemp;
		bufferTemp.a = bufferMain.a + dataPacket.a;

		if(bufferTemp.a <= 0)
			throw "bufferTemp.a <= 0";

		bufferTemp.b = new char[bufferTemp.a];

		memcpy(bufferTemp.b, bufferMain.b, bufferMain.a); //append old data
		memcpy(bufferTemp.b + bufferMain.a, dataPacket.b, dataPacket.a); //append new data
		delete[] bufferMain.b; //delete old buffer
		bufferMain = bufferTemp; //update with new buffer
	}

	while( true )
	{
		//stop if there is no message header available yet
		if(bufferMain.a < sizeof(M))
			break;

		M* m = (M*)bufferMain.b; //get message id
		unsigned int messageSize = M::struct_size[m->id]; //get message size using message id

		//stop if there is no message body available yet
		if( bufferMain.a < messageSize )
			break;

		//copy message from message data and push to list
		listM.push_back( m->clone() );

		//update main buffer
		Pair<unsigned int, char*> bufferTemp;
		bufferTemp.a = bufferMain.a - messageSize; //calculate new size
		bufferTemp.b = 0;

		if( bufferTemp.a != 0 )
		{
			//there is left over bytes from the next message in the buffer
			//move up the left over bytes and update buffer
			bufferTemp.b = new char[bufferTemp.a]; //allocate new memory for new buffer
			memcpy(bufferTemp.b, bufferMain.b + messageSize, bufferTemp.a); //append leftover data to new buffer
		}
		
		delete[] bufferMain.b; //delete old buffer
		bufferMain = bufferTemp; //update with new buffer
	}
}

bool Client::isQuit()
{
	return receiver.isQuit || sender.isQuit;
}
