#pragma once

#include "Runnable.hpp"
#include "CircularBuffer.hpp"
#include "Pair.hpp"
#include "Semaphore.hpp"

#include <winsock2.h>

//todo give better name?
class Sender : public Runnable
{
private:	
	SOCKET socket;
	CircularBuffer<Pair<unsigned int, char*>> circularBuffer;
	Semaphore semaphore;

public:
	bool isQuit;

	void construct(SOCKET socket);
	void destruct();
	void run();
	bool isFull();
	void addItem( const Pair<unsigned int, char*>& datapair );
};
