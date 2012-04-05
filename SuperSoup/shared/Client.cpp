#include "Client.hpp"

#include <list>

void Client::construct( SOCKET socket )
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

void Client::destruct()
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

//does not block the caller
//todo optmize this
void Client::fastSend(const Message& message)
{
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

void Client::checkMessages()
{
	Pair<unsigned int, char*> bufferA;
	bufferA.a = 0;
	bufferA.b = 0;

	std::list<Message> listMessage;

	//construct messages from received network data packets
	while(true)
	{
		while(receiver.isEmpty())
			Thread::Sleep(100);

		if(receiver.isEmpty())
			throw "Client: receiver.isEmpty()";

		//---push data into the ram memory for this thread---

		Pair<unsigned int, char*> dataPacket = receiver.popItem(); //fetch new data

		//create a new buffer that can contain the old data and new data //todo improve this?
		Pair<unsigned int, char*> bufferB;
		bufferB.a = bufferA.a + dataPacket.a;

		if(bufferB.a <= 0)
			throw "Client: bufferB.a <= 0";

		bufferB.b = new char[bufferB.a];

		memcpy(bufferB.b, bufferA.b, bufferA.a); //append old data
		memcpy(bufferB.b + bufferA.a, dataPacket.b, dataPacket.a); //append new data
		delete[] bufferA.b; //delete old buffer
		bufferA = bufferB; //update with new buffer

		//stop if there is no message header available yet
		unsigned int headerSize = sizeof(Message::uint32) + sizeof(Message::ushort16);

		if( bufferA.a < headerSize )
			continue; //todo

		//construct message header for the new message
		Message newMessage;
		newMessage.recpientID = ((Message::uint32*)(&bufferA.b[0]))[0];
		newMessage.messageSize = ((Message::ushort16*)(&bufferA.b[sizeof(Message::uint32)]))[0];

		//stop if there is no message body available yet
		if( bufferA.a < headerSize + newMessage.messageSize )
			continue; //todo

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
			memcpy(bufferC.b, bufferA.b + headerSize - newMessage.messageSize, bufferC.a); //append leftover data
			delete[] bufferA.b; //delete old buffer
			bufferA = bufferC; //update with new buffer
		}

		//push to list for later message handling
		//listMessage.push_back(newMessage);
		printf("%s\n", newMessage.messageData);
	}
}
