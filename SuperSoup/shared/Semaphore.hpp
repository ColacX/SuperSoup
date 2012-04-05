#pragma once

class Semaphore
{
private:
	void* semaphoreHandle;

public:
	void construct(unsigned int start = 0, unsigned int max = 1);
	void destruct();
	void wait( unsigned int ms = 0xFFFFFFFF );
    void post( unsigned int number = 1 );
};
