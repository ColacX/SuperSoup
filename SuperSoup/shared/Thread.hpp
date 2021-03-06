#pragma once

#include "Runnable.hpp"

class Thread
{
private:
    static unsigned long __stdcall entryPoint(void* parameter);
    void* threadHandle;
	Runnable* runnable;

public:
	void construct(Runnable& runnable);
	void destruct();
	void start();

	static void Sleep(unsigned int ms);
};
