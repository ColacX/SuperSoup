#pragma once

#include "Thread.hpp"
#include "Receiver.hpp"
#include "Sender.hpp"
#include "Message.hpp"

#include <list>

//todo give better name?
class Client
{
private:
	SOCKET socket;
	Thread receiverThread;
	Thread senderThread;
	Pair<unsigned int, char*> bufferA;

public:
	Receiver receiver; //todo make private
	Sender sender; //todo make private

	std::list<Message> listMessage;

	void construct( SOCKET socket );
	void destruct();

	//does not block the caller	
	void fastSend(const Message& message);

	void pushMessages();
};
