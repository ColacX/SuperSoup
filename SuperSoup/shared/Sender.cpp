#include <winsock2.h>
#include <stdio.h>

#include "Sender.hpp"

#include "SharedMisc.hpp"

//todo give better name?
void Sender::construct(SOCKET socket)
{
	this->socket = socket;
	isQuit = false;

	unsigned int bufferCapacity = 1024;
	circularBuffer.construct(bufferCapacity);
	semaphore.construct(0, bufferCapacity);
}

void Sender::destruct()
{
	circularBuffer.destruct();
	semaphore.destruct();
}

void Sender::run()
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

			Pair<unsigned int, char*> datapair = circularBuffer.popItem();
			int dataLength = datapair.a;
			char* dataPointer = datapair.b;
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

bool Sender::isFull()
{
	return circularBuffer.isFull();
}

//remember to only send deletable data
void Sender::addItem( const Pair<unsigned int, char*>& datapair )
{
	if(isFull())
		throw "GameServer: buffer isFull";

	circularBuffer.addItem(datapair);

	//signal buffer has item
	semaphore.post();
}
