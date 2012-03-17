//a windows window that supports opengl

#pragma once

#include <windows.h>
#include "../external/include/gl/gl.h" //additonal dependencies opengl32.lib must be included after windows.h

#include "../shared/SingleLinkedList.h"
#include "WindowListener.h"
#include "KeyboardListener.h"
#include "MouseListener.h"

class Window{

protected:
    LPCSTR wClassName; //needed for windows to identify the class
    LPCSTR wTitle;
    WNDCLASS wClass; //weird windows abstraction of something
	int px, py;
    unsigned int width,height;
    int colorDepth;
    bool isFullscreen;
    bool isVisible;
    HWND wHandle; //weird window abstraction of a handle to this window
    HDC wDeviceContext; //weird windows abstraction of a device
    HINSTANCE hInstance; //reference to calling process or something like that
    SingleLinkedList<WindowListener*> listWindowListener;
    SingleLinkedList<KeyboardListener*> listKeyboardListener;
    SingleLinkedList<MouseListener*> listMouseListener;
    
    static LRESULT CALLBACK wDefaultEventmanager(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam); //static default window event processor must be static
    static SingleLinkedList<Window*> listWindow; //static list of windows
    
public:

    Window(bool isFullscreen, char* windowName);
    ~Window();
    void create();
    void run();
    
    void swapBuffers();
    void addWindowListener(WindowListener* windowListener);
    void addKeyboardListener(KeyboardListener* oKeyboardListener);
    void addMouseListener(MouseListener* oMouseListener);
    void notifyEvent(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
    
    HDC getDeviceContext();
    HWND getWindowHandle();
    
    bool isFocused();
    
    void setTitle( char* newTitle );
    void setVisible( bool isVisible );
    void setFocused( bool isFocused );
    void setBounds( int x, int y, int w, int h );
	void setMaximized();
	void setNormalized();
	void setSize( unsigned int w, unsigned int h );

    unsigned int getWidth();
    unsigned int getHeight();
};