#pragma once

#include <list>

#include "Thread.hpp"
#include "Receiver.hpp"
#include "Sender.hpp"
#include "MessageSystem.h"

//todo give better name?
class Client
{
private:
	SOCKET connectionSocket;
	Thread receiverThread;
	Thread senderThread;
	Pair<unsigned int, char*> bufferMain;

public:
	Receiver receiver; //todo make private
	Sender sender; //todo make private

	std::list<M*> listM;

	static SOCKET connectTo(const char* targetIP, const char* targetPort);

	void construct( SOCKET connectionSocket );
	void destruct();
	
	void send(const M* m);
	void run();

	bool isQuit();
};
