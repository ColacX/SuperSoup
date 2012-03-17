//idk what to comment on here lulz

#pragma once

class WindowListener{
    protected:
        WindowListener();
        ~WindowListener();
    
    public:        
        virtual void windowResized(int width, int height);
        virtual void windowClosed();
        virtual void windowUnfocused();
        
        /*
        virtual void windowResized()=0;
        virtual void windowMoved()=0;
        virtual void windowMinimized()=0;
        virtual void windowMaximized()=0;
        virtual void windowRestored()=0;
        virtual void windowClosed()=0;
        virtual void windowGainedFocus()=0;
        virtual void windowLostFocus()=0;
        virtual void windowMousePressed()=0;
        virtual void windowMouseReleased()=0;
        */
};