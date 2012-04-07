#include <stdio.h>

#include "Receiver.hpp"

#include "Semaphore.hpp"
#include "CircularBuffer.hpp"
#include "Pair.hpp"
#include "Message.hpp"

#include "SharedMisc.hpp"

void Receiver::construct(SOCKET socket)
{
	this->socket = socket;

	isQuit = false;

	unsigned int bufferCapacity = 1024;
	circularBuffer.construct(bufferCapacity);
	semaphore.construct(bufferCapacity, bufferCapacity);

	bufferSize = 2048;
	bufferReceive = new char[bufferSize];
}

void Receiver::destruct()
{
	delete[] bufferReceive;
	circularBuffer.destruct();
}

void Receiver::run()
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
				throw "circularBuffer.isFull()";

			//fetch data from network into local buffer
			unsigned int receiveCount = 0;
			int recvR = recv( socket, bufferReceive, bufferSize, 0);

			if( recvR > 0 )
				receiveCount += recvR; //add receviced bytes
			else if( recvR == 0 )
				throw "recv connection closed";
			else
				throw "recv failed";

			//todo add check if list is full

			//push received network data to ram memory
			char* dataPointer = new char[receiveCount];
			memcpy( dataPointer, bufferReceive, receiveCount );

			/*
			for(unsigned int i=0; i<receiveCount; i++)
			{
				printf("%d\n", dataPointer[i]);
			}
			*/

			Pair<unsigned int, char*> datapair;
			datapair.a = receiveCount;
			datapair.b = dataPointer;
			circularBuffer.addItem( datapair );
		}
	}
	catch(char* ex)
	{
		//todo handle errors / disconnects
		printf("%s\n", ex);
	}
}

bool Receiver::isEmpty()
{
	return circularBuffer.isEmpty();
}

Pair<unsigned int, char*> Receiver::popItem()
{
	if( isEmpty() )
		throw "buffer isEmpty";
		
	Pair<unsigned int, char*> datapair = circularBuffer.popItem();

	//signal buffer has free slot
	semaphore.post();
		
	return datapair;
}
