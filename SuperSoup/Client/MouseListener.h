#pragma once

class MouseListener{

protected:
    MouseListener();
    ~MouseListener();

public:
	enum { BUTTON_LEFT, BUTTON_RIGHT };

    virtual void mousePressed( unsigned int button, int localX, int localY);
};