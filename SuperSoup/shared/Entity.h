#pragma once

#include "Message.hpp"

class Entity
{
public:
	virtual void construct();
	virtual void destruct();

	virtual void run();
	virtual void draw();

	virtual Message getSync();
};