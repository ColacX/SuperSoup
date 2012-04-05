#pragma once

#include "Runnable.hpp"
#include "Semaphore.hpp"
#include "CircularBuffer.hpp"
#include "Pair.hpp"

#include <WinSock2.h>

//todo give better name?
class Receiver : public Runnable
{
private:
	SOCKET socket;
	CircularBuffer<Pair<unsigned int, char*>> circularBuffer;
	Semaphore semaphore;
	unsigned int bufferSize;
	char* bufferReceive;

public:
	bool isQuit;

	void construct(SOCKET socket);
	void destruct();
	void run();
	bool isEmpty();
	Pair<unsigned int, char*> popItem();
};
