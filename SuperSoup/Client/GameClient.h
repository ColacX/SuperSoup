#pragma once

#include <stdio.h>
#include <math.h>
#include <gl\glew.h>
#include <gl\gl.h>
#include <Box2D\Box2D.h>

#include "..\shared\SingleLinkedList.h"
#include "..\shared\Entity.h"
#include "..\shared\Window.h"
#include "..\shared\Client.hpp"

#include "../shared/Ground.h"

class GameClient: public WindowListener, public KeyboardListener, public MouseListener{

private:
	bool isRunning;
	uint32 clientFramecount;
	bool keydown[256];
	int mouseCenterX;
	int mouseCenterY;    
	Window* window0;
	void checkControls();
	void setVerticalSync(bool sync);
	bool ingame;
	int gameWidth;
	int gameHeight;

	b2Body* player;
	Ground* ground;

	Entity* playerXXX;
	Client* clientXXX;

	int mx;
	int my;
public:
	char* targetIP;
	char* targetPort;

	GameClient();
	~GameClient();

	void run();
	void run2();

	void windowClosed();
	void windowResized(int width, int height);
    void windowUnfocused();
	void keyPressed(unsigned int virtualKeyCode);
    void keyReleased(unsigned int virtualKeyCode);
    void mousePressed( unsigned int button, int localX, int localY);
};

