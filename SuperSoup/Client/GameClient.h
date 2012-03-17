#pragma once

#include <stdio.h>
#include <math.h>

#include "../external/include/gl/glew.h"
#include "../external/include/gl/gl.h"
#include "../shared/SingleLinkedList.h"

#include "Window.h"

class GameClient: public WindowListener, public KeyboardListener, public MouseListener{

private:
	bool isRunning;
	unsigned int framecount;
	bool keydown[256];
	int mouseCenterX;
	int mouseCenterY;    
	Window* window0;
	void checkControls();
	void setVerticalSync(bool sync);
	bool ingame;

public:
	GameClient();
	~GameClient();

	void run();

	void windowClosed();
	void windowResized(int width, int height);
    void windowUnfocused();
	void keyPressed(unsigned int virtualKeyCode);
    void keyReleased(unsigned int virtualKeyCode);
    void mousePressed( unsigned int button, int localX, int localY);
    
};

