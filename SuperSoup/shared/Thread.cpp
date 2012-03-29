#include "Thread.hpp"

#include <Windows.h>

DWORD WINAPI Thread::entryPoint(LPVOID parameter)
{
    Runnable* runnable = (Runnable*)parameter;
    runnable->run();
    return 666;
}

void Thread::construct(Runnable& runnable)
{
	this->runnable = &runnable;
}

void Thread::destruct()
{
	int iResult = WaitForSingleObject( threadHandle, INFINITE);
    TerminateThread( threadHandle, 0 );
    CloseHandle( threadHandle );
}

void Thread::start()
{
	unsigned long threadID;
	threadHandle = CreateThread( 0, 0, entryPoint, runnable, 0, &threadID);
}

void windows_sleep(unsigned int ms)
{
	Sleep(ms);
}

void Thread::Sleep(unsigned int ms)
{
	windows_sleep(ms);
}