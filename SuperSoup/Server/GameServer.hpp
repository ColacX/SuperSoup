#pragma once

#include "..\shared\Window.h"

class GameServer : public WindowListener
{
private:
	Window* window0;
public:
	void run();
	void windowResized(int width, int height);
};