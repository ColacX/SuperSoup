#pragma once

class KeyboardListener{
    protected:
        KeyboardListener();
        ~KeyboardListener();

    public:
        virtual void keyPressed(unsigned int virtualKeyCode); //key is pressed
        virtual void keyReleased(unsigned int virtualKeyCode); //key is released
        //keyunicode16? what character in unicode
};