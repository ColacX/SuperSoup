#include "Semaphore.hpp"

#include <Windows.h>

void Semaphore::construct(unsigned int start, unsigned int max)
{
	semaphoreHandle = CreateSemaphore( NULL, start, max, NULL);

	if (semaphoreHandle == NULL) 
		throw "Semaphore: CreateSemaphore failed";
}

void Semaphore::destruct()
{
	CloseHandle(semaphoreHandle);
}

void Semaphore::wait( unsigned int ms)
{
	DWORD waitResult = WaitForSingleObject( semaphoreHandle, ms );

	if(waitResult == WAIT_OBJECT_0)
		; //wait success
	else if(waitResult == WAIT_TIMEOUT)
		throw "Semaphore: waitResult == WAIT_TIMEOUT";
	else
		throw "Semaphore: WaitForSingleObject failed";
}

void Semaphore::post( unsigned int number)
{
    if ( !ReleaseSemaphore(semaphoreHandle, number, NULL) )
		throw "Semaphore: ReleaseSemaphore failed";
}
