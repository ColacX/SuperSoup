#pragma comment(lib, "Box2D.lib" ) //box2d physics engine library
#pragma comment(lib, "opengl32.lib") //opengl library for 32-bit computers
#pragma comment(lib, "glu32.lib") //openglu32 librarary extension to opengl
#pragma comment(lib, "glew32.lib") //glew librarary extension to opengl
#pragma comment(lib, "Ws2_32.lib" ) //windows sockets v2 32 bit tcp-network library

#include <winsock2.h>
#include <ws2tcpip.h>
#include <Box2D/Box2D.h>
#include <cstdio>
#include <list>

#include "GameClient.h"
#include "GASimulation.h"

#include "..\shared\SharedMisc.hpp"

enum Mode
{
	Mode_Default,
	Mode_Network,
	Mode_GA_Simulation
};

int main(int argc, char** argv)
{
	//MemoryDebug::isEnabled = true;

	try
	{
		GameClient gc;
		GASimulation gas;
		Mode mode = Mode_GA_Simulation;

		for(int i=0; i<argc; i++)
		{
			printf("arg\t%i:\t%s\n", i, argv[i]);
		}

		for(int i=0; i<argc; i++)
		{
			if( strcmp(argv[i], "/network") == 0 )
			{
				mode = Mode_Network;
			}
			else if( strcmp(argv[i], "/targetIP") == 0 )
			{
				i++;
				gc.targetIP = argv[i];
			}
			else if( strcmp(argv[i], "/targetPort") == 0 )
			{
				i++;
				gc.targetPort = argv[i];
			}
		}

		switch( mode)
		{
		case Mode_Default:
			gc.run();
			break;

		case Mode_Network:
			gc.run2();
			break;

		case Mode_GA_Simulation:
			gas.run();
			break;
		}
	}
	catch( char* ex )
	{
		printf( "%s\n", ex );
		getchar();
	}

	//memoryDebug.debugPrint();
	//MemoryDebug::isEnabled = false;
	return 1337;
}
