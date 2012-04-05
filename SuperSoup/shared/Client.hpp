#pragma once

#include "Thread.hpp"
#include "Receiver.hpp"
#include "Sender.hpp"
#include "Message.hpp"

//todo give better name?
class Client
{
private:
	SOCKET socket;
	Thread receiverThread;
	Thread senderThread;

public:
	Receiver receiver;
	Sender sender;

	void construct( SOCKET socket );
	void destruct();

	//does not block the caller	
	void fastSend(const Message& message);

	void checkMessages();
};
