#pragma comment(lib, "Ws2_32.lib" ) //windows sockets v2 32 bit tcp-network library

#include "GameServer.hpp"

int main(int argc, char** argv)
{
	try
	{
		GameServer gs;
		gs.run();
	}
	catch(char* ex)
	{
		printf("%s\n", ex);
		getchar();
	}

	return 1337;
}