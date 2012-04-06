#pragma once

#include <list>

#include "Thread.hpp"
#include "Receiver.hpp"
#include "Sender.hpp"
#include "Message.hpp"

//todo give better name?
class Client
{
private:
	SOCKET connectionSocket;
	Thread receiverThread;
	Thread senderThread;
	Pair<unsigned int, char*> bufferA;

public:
	Receiver receiver; //todo make private
	Sender sender; //todo make private

	std::list<Message> listMessage;

	void construct( SOCKET connectionSocket );
	void destruct();

	void fastSend(const Message& message);
	void pushMessages();

	static SOCKET connectTo(const char* targetIP, const char* targetPort);
};
