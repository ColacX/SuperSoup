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
	message.recpientID = 0;
	message.messageSize = 0;
	message.messageData = 0;	
	return message;
}
