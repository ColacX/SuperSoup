#include <list>
#include <ws2tcpip.h>
#include <winsock2.h>

#include "Client.hpp"

#include "SharedMisc.hpp"

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

	bufferA.a = 0;
	bufferA.b = 0;
}

void Client::destruct()
{
	//free socket
	closesocket(connectionSocket);
		
	//wait for threads to stop
	receiver.isQuit = true;
	sender.isQuit = true;

	receiverThread.destruct();
	senderThread.destruct();

	//free memory
	receiver.destruct();
	sender.destruct();
}

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

std::list<Message> listrecording;

void Client::playback()
{
	for(auto it = listrecording.begin(); it != listrecording.end(); it++)
	{
		Message& message = *it;
		Message r = message;
		r.messageData = new char[r.messageSize];
		memcpy(r.messageData, message.messageData, r.messageSize);

		fastSend(r);
	}
}

//does not block the caller
//todo optmize this
void Client::fastSend(const Message& message)
{
	static bool playbackRecord = true;
	
	if(playbackRecord)
	{
		Message r = message;
		r.messageData = new char[r.messageSize];
		memcpy(r.messageData, message.messageData, r.messageSize);
		listrecording.push_back(r);
	}

	if(message.messageSize > 0 )
		int xxx = 2; //good for debugging todo remove this

	//printf("send message recipent:%d size: %d\n", message.recpientID, message.messageSize);

	//send recipentID
	{
		unsigned int s = sizeof(message.recpientID); //size of recipent id
		char* p = new char[s];
		memcpy( p,
			(char*)&message.recpientID //pointer to recipent id
			, s);

		Pair<unsigned int, char*> datapair;
		datapair.a = s;
		datapair.b = p;
		sender.addItem(datapair);
	}

	//send message size
	{
		unsigned int s = sizeof(message.messageSize); //size of message size
		char* p = new char[s];
		memcpy( p,
			(char*)&message.messageSize //pointer to message size
			, s);

		Pair<unsigned int, char*> datapair;
		datapair.a = s;
		datapair.b = p;
		sender.addItem(datapair);
	}

	//send message data
	{
		Pair<unsigned int, char*> datapair;
		datapair.a = message.messageSize; //size of message
		datapair.b = (char*)message.messageData; //pointer to message
		sender.addItem(datapair);
	}
}

//construct messages from received network data packets
//push data into the ram memory for this thread
//todo improve this buggy shit!
void Client::pushMessages()
{
	Pair<unsigned int, char*> dataPacket; //fetch new data
	dataPacket.a = 0;
	dataPacket.b = 0;

	while( !receiver.isEmpty() )
	{
		//fetch new data
		dataPacket = receiver.popItem(); 

		//printf("packet size: %d\n", dataPacket.a);

		//create a new buffer that can contain the old data and new data //todo improve this?
		Pair<unsigned int, char*> bufferB;
		bufferB.a = bufferA.a + dataPacket.a;

		if(bufferB.a <= 0)
			throw "bufferB.a <= 0";

		bufferB.b = new char[bufferB.a];

		memcpy(bufferB.b, bufferA.b, bufferA.a); //append old data
		memcpy(bufferB.b + bufferA.a, dataPacket.b, dataPacket.a); //append new data
		delete[] bufferA.b; //delete old buffer
		bufferA = bufferB; //update with new buffer
	}

	//stop if there is no message header available yet
	unsigned int headerSize = sizeof(Message::uint32) + sizeof(Message::ushort16);

	while( bufferA.a >= headerSize )
	{

		//construct message header for the new message
		Message newMessage;
		newMessage.recpientID = ((Message::uint32*)(&bufferA.b[0]))[0];
		newMessage.messageSize = ((Message::ushort16*)(&bufferA.b[sizeof(Message::uint32)]))[0];

		if(newMessage.messageSize > 1024)
		{
			int x = 2; //for debugging wrong messages
		}

		//stop if there is no message body available yet
		if( bufferA.a < headerSize + newMessage.messageSize )
		{
			return;
		}

		//construct message
		newMessage.messageData = (Message::byte8*)new char[newMessage.messageSize]; //allocate message memory
		memcpy(newMessage.messageData, bufferA.b + headerSize, newMessage.messageSize); //copy only the message data

		//update buffer
		Pair<unsigned int, char*> bufferC;
		bufferC.a = bufferA.a - headerSize - newMessage.messageSize;
		bufferC.b = 0;

		//if there is left over bytes from the next message in the buffer
		if( bufferC.a != 0 )
		{
			//move up the left over bytes and update buffer
			bufferC.b = new char[bufferC.a];
			memcpy(bufferC.b, bufferA.b + headerSize + newMessage.messageSize, bufferC.a); //append leftover data
			delete[] bufferA.b; //delete old buffer
			bufferA = bufferC; //update with new buffer
		}
		else
		{
			delete[] bufferA.b; //delete old buffer
			bufferA.a = 0;
			bufferA.b = 0;
		}

		//push to list for later message handling
		//printf("bufferA: %d\n", bufferA.a);
		//printf("bufferC: %d\n", bufferC.a);
		//printf("received message recipent:%d size: %d\n", newMessage.recpientID, newMessage.messageSize);
		listMessage.push_back(newMessage);
	}
}
