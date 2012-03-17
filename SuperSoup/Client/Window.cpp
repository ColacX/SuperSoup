#pragma once

#include "Window.h"
#include <stdio.h>

SingleLinkedList<Window*> Window::listWindow; //static list of windows

Window::Window(bool isFullscreen, char* windowName){
    width = 400;
    height = 400;
    colorDepth = 32;
    this->isFullscreen = isFullscreen;
    wClassName = windowName;

    wTitle = windowName;
    hInstance = GetModuleHandle(0); //get calling process handle maybee

    Window::listWindow.add(this);

    isVisible = false;
    wHandle = 0;
    wDeviceContext = 0;
    ZeroMemory( &wClass, sizeof(wClass) );
}

Window::~Window(){
    if(this->isFullscreen){
        ChangeDisplaySettings(0,0); // if so switch back to the desktop
        ShowCursor(true); // show mouse pointer
    }
    this->isFullscreen = false;

    if(this->wDeviceContext != 0){
        ReleaseDC(this->wHandle, this->wDeviceContext);
    }
    this->wDeviceContext = 0;

    if(this->wHandle != 0){
        DestroyWindow(this->wHandle);
    }
    this->wHandle = 0;

    UnregisterClass(this->wClassName,this->hInstance);
    this->wClassName = 0;
    this->hInstance = 0;

    ZeroMemory( &this->wClass, sizeof(this->wClass) );

    this->isVisible = false;

    for(unsigned int i=0; i<Window::listWindow.getCount(); i++){
        Window* window = Window::listWindow.get(i);
        if(window == this){
            Window::listWindow.remove(i);
            break;
        }
    }
}

//prepare window for usage
//window message listening thread must own the device context or they'll be ignored
void Window::create(){
    //thread must own the device context or all window messages will be unhandled
    //device context should be owned by the window listening thread
    wClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC; // Redraw On Size, And Own DC For Window.
    wClass.lpfnWndProc = (WNDPROC)Window::wDefaultEventmanager;
    wClass.cbClsExtra = 0; // No Extra Window Data
    wClass.cbWndExtra = 0; // No Extra Window Data
    wClass.hInstance = hInstance; // Set The Instance
    wClass.hIcon = LoadIcon(0, IDI_WINLOGO); // Load The Default Icon
    wClass.hCursor = LoadCursor(0, IDC_ARROW); // Load The Arrow Pointer
    wClass.hbrBackground = 0; // No Background RequicolorR For GL
    wClass.lpszMenuName = 0; // We Don't Want A Menu
    wClass.lpszClassName = wClassName; // Set The Class Name

    if( !RegisterClass(&wClass) ){
        throw "Window: RegisterClass failed";
    }

    DWORD dwExStyle; // Window Extended Style
    DWORD dwStyle; // Window Style

    RECT wRectangle; // Grabs Rectangle Upper Left / Lower Right Values
    wRectangle.left=(long)0; // Set Left Value To 0
    wRectangle.right=(long)width; // Set Right Value To Requested Width
    wRectangle.top=(long)0; // Set Top Value To 0
    wRectangle.bottom=(long)height; // Set Bottom Value To Requested Height

    if(this->isFullscreen){
        DEVMODE dmScreenSettings; // Device Mode
        memset( &dmScreenSettings, 0, sizeof(dmScreenSettings) ); // Makes Sure Memory's CleacolorR
        dmScreenSettings.dmSize = sizeof(dmScreenSettings); // Size Of The Devmode Structure
        dmScreenSettings.dmPelsWidth = this->width; // Selected Screen Width
        dmScreenSettings.dmPelsHeight = this->height; // Selected Screen Height
        dmScreenSettings.dmBitsPerPel = this->colorDepth; // Selected Bits Per Pixel
        dmScreenSettings.dmFields = DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;

        // Try To Set Selected Mode And Get Results.  NOTE: CDS_fullscreen Gets Rid Of Start Bar.
        if( ChangeDisplaySettings(&dmScreenSettings,CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL){
            throw "Window: ChangeDisplaySettings failed";
        }
        else{
            dwExStyle = WS_EX_APPWINDOW; // Window Extended Style
            dwStyle = WS_POPUP; // Windows Style
        }
    }
    else{
        dwExStyle = WS_EX_APPWINDOW|WS_EX_WINDOWEDGE; // Window Extended Style
        dwStyle = WS_OVERLAPPEDWINDOW; // Windows Style
    }

    AdjustWindowRectEx(&wRectangle, dwStyle, false, dwExStyle); // Adjust Window To True Requested Size

    this->wHandle = CreateWindowEx( dwExStyle, // Extended Style For The Window
                                    wClassName, // Class Name
                                    wTitle, // Window Title
                                    dwStyle | // Defined Window Style
                                    WS_CLIPSIBLINGS | WS_CLIPCHILDREN, // RequicolorR Window Style
                                    0, 0, // Window Position
                                    wRectangle.right-wRectangle.left, // Calculate Window Width
                                    wRectangle.bottom-wRectangle.top, // Calculate Window Height
                                    NULL, // No Parent Window
                                    NULL, // No Menu
                                    hInstance, // Instance
                                    NULL); // Dont Pass Anything To WM_CREATE
    if(this->wHandle == 0){
        throw "Window: CreateWindowEx failed";
    }

    if(!(this->wDeviceContext = GetDC(this->wHandle))){
        throw "Window: GetDC failed";
    }

    this->setVisible(true);

    PIXELFORMATDESCRIPTOR pixelformatDescriptor = { // pfd Tells Windows How We Want Things To Be
	    sizeof(PIXELFORMATDESCRIPTOR), // Size Of This Pixel Format Descriptor
	    1, // Version Number
	    PFD_DRAW_TO_WINDOW | // Format Must Support Window
	    PFD_SUPPORT_OPENGL | // Format Must Support OpenGL
        PFD_DOUBLEBUFFER, // Must Support Double Buffering
	    PFD_TYPE_RGBA, // Request An RGBA Format
	    this->colorDepth, // Select Our Color Depth
	    0, 0, 0, 0, 0, 0, // Color Bits IgnocolorR
	    0, // No Alpha Buffer
	    0, // Shift Bit IgnocolorR
	    0, // No Accumulation Buffer
	    0, 0, 0, 0, // Accumulation Bits IgnocolorR
	    24, // 24Bit Z-Buffer (Depth Buffer)
	    0, // No Stencil Buffer //TODO turn this shit on !
	    0, // No Auxiliary Buffer
	    PFD_MAIN_PLANE, // Main Drawing Layer
	    0, // Reserved
	    0, 0, 0 // Layer Masks IgnocolorR
    };

    //windows will choose a pixel format thats as close as possible to what we wanted
    int rChoosePixelFormat = ChoosePixelFormat( this->wDeviceContext, &pixelformatDescriptor);
    if( rChoosePixelFormat == 0 ){
        throw "Window: ChoosePixelFormat failed";
    }

    if(!SetPixelFormat( this->wDeviceContext, rChoosePixelFormat, &pixelformatDescriptor)){
	    throw "Window: SetPixelFormat failed";
    }
}

void Window::run(){
    MSG    wMessage;
    PeekMessage(&wMessage, 0, 0, 0, PM_REMOVE); //this peeks for messages and removes them
    DispatchMessage(&wMessage); //checks which window procedure the message belongs to and sends it to it
}

void Window::setVisible(bool isVisible){
    this->isVisible = isVisible;

    if(this->isVisible){
        ShowWindow(this->wHandle,SW_SHOW); // show the window
        SetForegroundWindow(this->wHandle); // give sligthly higher priority
        SetFocus(this->wHandle); // set OS focus to this window
    }
    else{
        ShowWindow(this->wHandle,SW_HIDE);
    }
}

//blocks for vertical-sync //swaps the screen buffer
void Window::swapBuffers(){
    SwapBuffers(this->wDeviceContext);
}

void Window::addWindowListener(WindowListener* windowListener){
    this->listWindowListener.add(windowListener);
}

void Window::addKeyboardListener(KeyboardListener* oKeyboardListener){
    this->listKeyboardListener.add(oKeyboardListener);
}

void Window::addMouseListener(MouseListener* oMouseListener){
    this->listMouseListener.add(oMouseListener);
}

HDC Window::getDeviceContext(){
    return this->wDeviceContext;
}

HWND Window::getWindowHandle(){
    return this->wHandle;
}

//this function must be static or it wont work
//this is the function that will be called whenever a dispatchmessage is called
LRESULT CALLBACK Window::wDefaultEventmanager(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
    //printf("window message %x %x %x\n",uMsg,wParam,lParam);

    switch(uMsg){
        case WM_SYSCOMMAND: //Check System Commands
            switch(wParam){ //Check System Calls
                case SC_SCREENSAVE: //screensaver trying to start?
                case SC_MONITORPOWER: //monitor trying to enter powersave?
                return 0; //prevent both from happening by skipping DefWindowProc
            }
    }

    for(unsigned int i=0; i<Window::listWindow.getCount(); i++){
        Window* window = Window::listWindow.get(i);
        if(window->getWindowHandle() == hWnd){
            window->notifyEvent(hWnd,uMsg,wParam,lParam);
            break;
        }
    }

    int returnValue = DefWindowProc(hWnd,uMsg,wParam,lParam); //pass to default window processor

    return returnValue;
}

//todo complete this
void Window::notifyEvent(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
    switch(uMsg){
        case WM_LBUTTONDOWN: //mouse left button
            for(unsigned int i=0; i<this->listMouseListener.getCount(); i++){
                this->listMouseListener.get(i)->mousePressed( MouseListener::BUTTON_LEFT, LOWORD(lParam), HIWORD(lParam) );
            }
            return;

		case WM_RBUTTONDOWN: //mouse left button
            for(unsigned int i=0; i<this->listMouseListener.getCount(); i++){
                this->listMouseListener.get(i)->mousePressed( MouseListener::BUTTON_RIGHT, LOWORD(lParam), HIWORD(lParam) );
            }
            return;

        case WM_KEYDOWN:
            for(unsigned int i=0; i<this->listKeyboardListener.getCount(); i++){
                this->listKeyboardListener.get(i)->keyPressed( wParam );
            }
            return;

        case WM_KEYUP:
            for(unsigned int i=0; i<this->listKeyboardListener.getCount(); i++){
                this->listKeyboardListener.get(i)->keyReleased( wParam );
            }
            return;

        case WM_SIZE: //window resize
            this->width = LOWORD(lParam);
            this->height = HIWORD(lParam);
            for(unsigned int i=0; i<this->listWindowListener.getCount(); i++){
                this->listWindowListener.get(i)->windowResized(this->width, this->height);
            }
            return;

        case WM_CLOSE: //window close
            for(unsigned int i=0; i<this->listWindowListener.getCount(); i++){
                this->listWindowListener.get(i)->windowClosed();
            }
            return;
        case WM_NCACTIVATE: //window unfocused
            for(unsigned int i=0; i<this->listWindowListener.getCount(); i++){
                this->listWindowListener.get(i)->windowUnfocused();
            }
            return;
    }
}

bool Window::isFocused(){
    HWND currentlyFocusedWindow = GetFocus();
    if(currentlyFocusedWindow = this->wHandle){
        return true;
    }
    else{
        return false;
    }
}

void Window::setFocused(bool isFocused){
    if(isFocused){
        HWND previouslyFocusedWindow = SetFocus(this->wHandle);
    }
    else{
        HWND previouslyFocusedWindow = SetFocus(0); //doesnt work?
        //todo force unfocus if false
    }
}

void Window::setTitle(char* newTitle){
    SetWindowText(this->wHandle, newTitle);
}

void Window::setBounds( int x, int y, int w, int h ){
	px = x;
	py = y;
	width = w;
	height = h;

	if( wHandle != 0 ){
		MoveWindow( wHandle, x, y, w, h, true );
	}
}

unsigned int Window::getWidth()
{
    return width;
}

unsigned int Window::getHeight()
{
    return height;
}

void Window::setMaximized(){
	ShowWindow( wHandle, SW_SHOWMAXIMIZED );
}

void Window::setNormalized(){
	ShowWindow( wHandle, SW_NORMAL );
}

void Window::setSize( unsigned int w, unsigned int h ){
	width = w;
	height = h;
}