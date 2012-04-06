#include "Entity.h"

#include "SharedMisc.hpp"

void Entity::construct()
{
}

void Entity::destruct()
{
}

void Entity::run()
{
}

void Entity::draw()
{
}

Message Entity::getSync()
{
	Message message;
	message.recpientID = 15;
	message.messageSize = 4;
	message.messageData = new char[4];
	message.messageData[0] = 'l';
	message.messageData[1] = 'o';
	message.messageData[2] = 'l';
	message.messageData[3] = 0;
	return message;
}
