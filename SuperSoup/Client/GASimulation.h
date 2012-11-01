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

class Plant;
class Console;

class GASimulation: public WindowListener, public KeyboardListener, public MouseListener{

private:
	bool isRunning;
	uint32 clientFramecount;
	void checkControls();
	void setVerticalSync(bool sync);
	int gameWidth;
	int gameHeight;
	bool keydown[256];
	int mouseCenterX;
	int mouseCenterY;
	Window* window0;
	Console* console;

public:
	GASimulation();
	~GASimulation();

	void TestPlant(Plant* plant);
	void run();
	void windowClosed();
	void windowResized(int width, int height);
    void windowUnfocused();
	void keyPressed(unsigned int virtualKeyCode);
    void keyReleased(unsigned int virtualKeyCode);
    void mousePressed( unsigned int button, int localX, int localY);
	void mouseReleased(unsigned int button, int localX, int localY);
};

