#include <Box2D\Box2D.h>
#include <cstdio>
#include <WinSock2.h>

#include "GASimulation.h"

#include "..\shared\Client.hpp"
#include "..\shared\Entity.h"

#include "..\shared\SharedMisc.hpp"

extern void DrawTransform(const b2Transform& xf);
extern void DrawPoint(const b2Vec2& p, float32 size, const b2Color& color);
extern void DebugDrawBox(const b2Body& body, const b2PolygonShape& polygonShape);
extern int round(float f);
extern void DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color);

//todo give better name?
class Console : public Runnable
{
private:

public:
	unsigned int bufferSize;
	char* bufferConsole;
	bool isQuit;
	bool isGraphic;
	bool isDebug;
	bool isVSync;

	void construct()
	{
		bufferSize = 1024;
		bufferConsole = new char[bufferSize];

		isQuit = false;
		isGraphic = false;
		isDebug = false;
		isVSync = true;
	}

	void destruct()
	{
		delete[] bufferConsole;
	}

	void run()
	{
		try
		{
			while(true)
			{
				if(isQuit)
					break; //quit

				ZeroMemory(bufferConsole, bufferSize);
				scanf_s("%s", bufferConsole, bufferSize);
				getchar(); //extra needed???

				if(strcmp(bufferConsole, "/help") == 0)
				{
					printf("available commands:\n");
					printf("/help\n");
					printf("/quit\n");
					printf("/graphic\n");
					printf("/debug\n");
					printf("/vsync\n");
				}
				else if(strcmp(bufferConsole, "/quit") == 0)
					isQuit = !isQuit;
				else if(strcmp(bufferConsole, "/graphic") == 0)
					isGraphic = !isGraphic;
				else if(strcmp(bufferConsole, "/debug") == 0)
					isDebug = !isDebug;
				else if(strcmp(bufferConsole, "/vsync") == 0)
					isVSync = !isVSync;
				else
					printf("Console: unknown command. Enter /help for available commands\n");
			}
		}
		catch(char* ex)
		{
			printf("%s\n", ex);
		}
	}
};

class GeneticAlgorithm
{
public:
	int score;
	std::list<char> genes;
	std::list<char> instinct;

	virtual void Mutate() = 0;

	void Save(char* filePath)
	{
	}

	void Load(char* filePath)
	{
	}

	void Run()
	{
	}

	void Draw()
	{
	}
};

class Leaf : public Entity
{
	void draw()
	{
		b2Vec2 position = body->GetPosition();
		b2Color color;

		if( body->IsAwake() )
			color = b2Color(0,1,0);
		else
			color = b2Color(0,0.5,0);

		auto transform = body->GetTransform();
		b2Vec2 vertices[b2_maxPolygonVertices];

		for (b2Fixture* f = body->GetFixtureList(); f; f = f->GetNext())
		{
			b2PolygonShape* polygonShape = (b2PolygonShape*)f->GetShape();

			for(int32 i=0; i<polygonShape->m_vertexCount; i++)
				vertices[i] = b2Mul( transform, polygonShape->m_vertices[i] );

			DrawPolygon( vertices, polygonShape->m_vertexCount, color );
		}
	}
};

class Wood : public Entity
{
	void draw()
	{
		b2Vec2 position = body->GetPosition();
		b2Color color;

		if( body->IsAwake() )
			color = b2Color(152.0f/256.0f, 118.0f/256.0f, 54.0f/256.0f);
		else
			color = b2Color(152.0f/256.0f/2.0f, 118.0f/256.0f/2.0f, 54.0f/256.0f/2.0f);

		auto transform = body->GetTransform();
		b2Vec2 vertices[b2_maxPolygonVertices];

		for (b2Fixture* f = body->GetFixtureList(); f; f = f->GetNext())
		{
			b2PolygonShape* polygonShape = (b2PolygonShape*)f->GetShape();

			for(int32 i=0; i<polygonShape->m_vertexCount; i++)
				vertices[i] = b2Mul( transform, polygonShape->m_vertices[i] );

			DrawPolygon( vertices, polygonShape->m_vertexCount, color );
		}
	}
};

class Plant : public GeneticAlgorithm
{
public:
	unsigned int growthProgress;
	unsigned int instinctProgress;
	std::list<Entity*> listBody;
	int delayCount;
	std::list<char>::iterator genesIterator;
	int direction;
	int currentBody;

	enum Instruction
	{
		GrowLeaf,
		GrowWood,
		DoNothing,
	};

	void Mutate()
	{
		//maybee add a new gene
		if(rand() % 2 == 0)
		{
			int r = rand() % 3;

			if(r == 0)
				genes.push_back( GrowLeaf );
			if(r == 1)
				genes.push_back( GrowWood );
			if(r == 2)
				genes.push_back( DoNothing );
		}

		//maybee modify
		if(rand() % 8 == 0 && genes.size() > 0)
		{
			auto it = genes.begin();
			std::advance( it, rand() % genes.size() );
			int r = rand() % 3;

			if(r == 0)
				*it = GrowLeaf;
			if(r == 1)
				*it = GrowWood;
			if(r == 2)
				*it = DoNothing;

			//delete any child nodes that follows the modification
			//todo improve this to delete only the associated nodes
			it++;
			genes.erase(it, genes.end());
		}

		/*
		//maybee delete an old gene
		if(rand() % 8 == 0 && genes.size() > 0)
		{
		auto it = genes.begin();
		std::advance( it, rand() % genes.size() );
		genes.erase(it);
		}
		*/

		if(genes.size() == 0)
			Mutate();
	}

	static Plant* Crossover(Plant& a, Plant& b)
	{
		Plant* p = new Plant;

		auto ia = a.genes.begin();
		auto ib = b.genes.begin();

		while(ia != a.genes.end() || ib != b.genes.end())
		{
			if(ia == a.genes.end())
				p->genes.push_back(*ib);
			else if(ib == b.genes.end())
				p->genes.push_back(*ia);
			else
			{
				if( rand() % 2 == 0)
					p->genes.push_back(*ia);
				else
					p->genes.push_back(*ib);
			}

			if(ia != a.genes.end()) 
				ia++;

			if(ib != b.genes.end()) 
				ib++;
		}

		return p;
	}

	void Reset()
	{
		growthProgress = 0;
		instinctProgress = 0;

		std::for_each(listBody.begin(), listBody.end(), [] (Entity* entity)
		{
			delete entity;
		});

		listBody.clear();

		delayCount = 0;
		direction = 0;
	}

	void Test()
	{
		genes.push_back(GrowWood);
		genes.push_back(DoNothing);
		genes.push_back(DoNothing);
		genes.push_back(DoNothing);
		genes.push_back(GrowWood);
		genes.push_back(DoNothing);
		genes.push_back(DoNothing);
		genes.push_back(DoNothing);
		genes.push_back(GrowWood);
		genes.push_back(DoNothing);
		genes.push_back(GrowWood);
		genes.push_back(GrowWood);
		genes.push_back(GrowWood);
		genes.push_back(DoNothing);
		genes.push_back(GrowLeaf);
		genes.push_back(GrowLeaf);
		genes.push_back(DoNothing);
		genes.push_back(DoNothing);
		genes.push_back(GrowLeaf);
		genes.push_back(DoNothing);
		genes.push_back(GrowLeaf);
		genes.push_back(GrowLeaf);
		genes.push_back(GrowLeaf);
		genes.push_back(GrowLeaf);
		genes.push_back(GrowLeaf);
	}

	void Join(b2World& world, Entity& a, Entity& b, int direction)
	{
		b2RevoluteJointDef jointDef;

		jointDef.bodyA = a.body;
		jointDef.bodyB = b.body;

		switch(direction)
		{
		case 0: //grow up
			jointDef.localAnchorA.Set(+0.0f, +0.5f);
			jointDef.localAnchorB.Set(+0.0f, -0.5f);
			break;
		case 1: //grow down
			jointDef.localAnchorA.Set(+0.0f, -0.5f);
			jointDef.localAnchorB.Set(+0.0f, +0.5f);
			break;
		case 2: //grow left
			jointDef.localAnchorA.Set(-0.5f, +0.0f);
			jointDef.localAnchorB.Set(+0.5f, +0.0f);
			break;
		case 3: //grow right
			jointDef.localAnchorA.Set(+0.5f, +0.0f);
			jointDef.localAnchorB.Set(-0.5f, +0.0f);
			break;
		}

		b2RevoluteJoint* joint = (b2RevoluteJoint*)world.CreateJoint(&jointDef);
		joint->EnableLimit(true);
	}

	void Grow(b2World& world, std::list<Entity*>& listEntity, Entity& ground, Entity& newEntity)
	{
		if(listBody.size() == 0)
		{
			//root node join with ground
			b2Vec2 p = ground.body->GetPosition();
			newEntity.positionY = p.y;
			newEntity.construct(world);
			Join(world, ground, newEntity, direction);
			listBody.push_back(&newEntity);
			listEntity.push_back(&newEntity);
			currentBody = 0;
		}
		else
		{
			//child node
			auto bodyIterator = listBody.begin();
			std::advance(bodyIterator, currentBody);

			b2Vec2 p = (*bodyIterator)->body->GetPosition();
			newEntity.positionX = p.x;
			newEntity.positionY = p.y;
			newEntity.construct(world);
			Join(world, **bodyIterator, newEntity, direction);
			listBody.push_back(&newEntity);
			listEntity.push_back(&newEntity);
		}
	}

	void Grow(b2World& world, std::list<Entity*>& listEntity, Entity& ground)
	{
		if(growthProgress == 0)
			genesIterator = genes.begin();

		//fetch and perform genetic instruction
		char instruction = *genesIterator;
		genesIterator++;
		growthProgress++;

		switch(instruction)
		{
		case GrowLeaf:
			{
				Leaf* leaf = new Leaf();
				Grow(world, listEntity, ground, *leaf);
				break;
			}
		case GrowWood:
			{
				Wood* wood = new Wood();
				Grow(world, listEntity, ground, *wood);
				break;
			}
		case DoNothing:
			{
				break;
			}
		}

		direction++;

		if(direction == 4)
		{
			//go to next child node after going one lap around
			if( listBody.size() != 0)
				currentBody = (currentBody + 1) % listBody.size();
			direction = 0;
		}
	}

	void Run(b2World& world, std::list<Entity*>& listEntity, Entity& ground)
	{
		delayCount++;

		if(delayCount % 10 != 0) //delay between growth so each box can develop completly in physics world
			return;

		//run uncompleted genetic instructions
		if(growthProgress < genes.size())
		{
			Grow(world, listEntity, ground);
			delayCount = 0;
		}
		else
		{
			if(delayCount == 120)
			{
				//kill any body parts that is below ground
				{
					auto ia = listBody.begin();

					while(ia != listBody.end())
					{
						auto prev = ia;
						ia++;

						if( (*prev)->body->GetPosition().y < 0.0f )
						{
							world.DestroyBody((*prev)->body);
							delete *prev;
							listEntity.remove(*prev);
							listBody.erase(prev);
							score -= 4;
						}
					}
				}

				//kill any body parts that is stuck together
				{
					auto ia = listBody.begin();

					while(ia != listBody.end())
					{
						auto prev = ia;
						ia++;

						auto ib = ia;
						auto pa = (*prev)->body->GetPosition();

						while(ib != listBody.end())
						{
							auto pb = (*ib)->body->GetPosition();
							auto d = b2Distance(pa, pb);

							if( d < 0.8f )
							{
								world.DestroyBody((*prev)->body);
								delete *prev;
								listEntity.remove(*prev);
								listBody.erase(prev);
								score -= 4;
								break;
							}

							ib++;
						}
					}
				}
			}
		}
	}

	void Draw()
	{
		std::for_each(listBody.begin(), listBody.end(), [] (Entity* entity)
		{
			entity->draw();
		});
	}
};

class Animal : public GeneticAlgorithm
{
public:
	void Grow()
	{
	}

	void Run()
	{
	}

	void Draw()
	{
	}
};

class PopulationPool
{
public:
	std::vector<Plant*> listPopulation;
	unsigned int generationCounter;

	void SelectTop(unsigned int topNumber)
	{
		if(topNumber >= listPopulation.size())
			return;

		std::sort(listPopulation.begin(), listPopulation.end(),
			[](Plant* a, Plant* b)
		{
			return a->score > b->score;
		}
		);

		auto it = listPopulation.begin();
		std::advance(it, topNumber);

		auto its = it;

		std::for_each(it, listPopulation.end(), 
			[] (Plant* plant)
		{
			delete plant;
		}
		);

		listPopulation.erase(it, listPopulation.end());
	}

	void Grow(unsigned int maxPopulation)
	{
		if( listPopulation.size() <= 0 )
			return;

		while(listPopulation.size() < maxPopulation)
		{
			auto baby = Plant::Crossover(*listPopulation[rand() % listPopulation.size()], *listPopulation[rand() % listPopulation.size()]);
			baby->Mutate();
			listPopulation.push_back(baby);
		}

		generationCounter++;
	}

	void Save(char* filePath)
	{
	}

	void Load(char* filePath)
	{
		Plant* plant = new Plant;
		listPopulation.push_back( plant );
		generationCounter = 0;
	}
};

GASimulation::GASimulation()
{
	isRunning = false;
	clientFramecount = 0;
}

GASimulation::~GASimulation()
{
	isRunning = false;
	//wait for run to stop?
}

void GASimulation::setVerticalSync(bool sync)
{
	typedef bool (APIENTRY *PFNWGLSWAPINTERVALFARPROC)(int);
	PFNWGLSWAPINTERVALFARPROC wglSwapIntervalEXT = 0;
	wglSwapIntervalEXT = (PFNWGLSWAPINTERVALFARPROC)wglGetProcAddress("wglSwapIntervalEXT");
	wglSwapIntervalEXT(sync);
}

void GASimulation::windowClosed()
{
	isRunning = false;
}

void GASimulation::windowResized(int width, int height)
{
	gameWidth = width;
	gameHeight = height;

	glViewport(0, 0, gameWidth, height);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, gameWidth/20.0f,-gameHeight/20.0f, 0);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glTranslatef(+gameWidth/40.0f, -gameHeight/40.0f, 0);
}

void GASimulation::windowUnfocused()
{
	while( ShowCursor(true)<0 ){}
	this->mouseCenterX = 0;
	this->mouseCenterY = 0;
	ZeroMemory(this->keydown,256);
}

void GASimulation::keyPressed(unsigned int virtualKeyCode)
{
}

void GASimulation::keyReleased(unsigned int virtualKeyCode)
{
}

void GASimulation::mousePressed( unsigned int button, int localX, int localY)
{
}

void GASimulation::mouseReleased(unsigned int button, int localX, int localY)
{
}

void GASimulation::checkControls()
{

}

void GASimulation::TestPlant(Plant* plant)
{
	//define the gravity vector.
	//construct a world object, which will hold and simulate the rigid bodies.
	b2Vec2 gravity(0.0f, -10.0f);
	b2World* world = new b2World(gravity);
	world->SetAllowSleeping(true);
	std::list<Entity*> listEntity;

	//create game objects
	Entity* ground = new Entity;
	ground->bodyType = b2_staticBody;
	ground->shapeWidth = 1000;
	ground->construct(*world);	
	listEntity.push_back(ground);

	for(int i=0; i<60*10; i++)
	{
		//if(plantNumber == 21 && i == 39)
		//printf("plant frame number: %d\n", i);

		//check user interactions
		{
			if(console->isGraphic)
			{
				for(int i=0; i<5; i++)
					window0->run();

				checkControls();

				setVerticalSync(console->isVSync);
			}

			if(console->isQuit)
				break;
		}

		//run game objects simulation
		{
			plant->Run(*world,listEntity,*ground);
			world->Step(1.0f/60.0f, 6, 2);
		}

		if(console->isGraphic)
		{
			//reset drawing buffer
			{
				glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
			}

			//move camera view into correct position
			{
				glLoadIdentity();
				glTranslatef(+gameWidth/40.0f, -gameHeight/40.0f, 0);
				//glTranslatef(-player->GetPosition().x,-player->GetPosition().y,0.0f);
				//printf("x:%+e\ty:%+e\n", player->GetPosition().x, player->GetPosition().y);
			}

			//draw game objects
			{
				//origo
				DrawPoint(b2Vec2(0,0), 3.0f, b2Color(0.0f,1.0f,0.0f));

				std::for_each(listEntity.begin(), listEntity.end(), [](Entity* entity)
				{
					entity->draw();
				});

				window0->swapBuffers();
			}
		}

		//sleep //some kind of syncing is necessary
		//Sleep(1000/60);
		clientFramecount++;
		//printf("client frame count %d\n", clientFramecount);
	}

	for(auto it = plant->listBody.begin(); it != plant->listBody.end(); it++)
	{
		if( dynamic_cast<Leaf*>(*it) )
			plant->score++;
	}

	plant->score += plant->listBody.size();

	if(console->isDebug)
		printf("    score: %d\n", plant->score);

	delete world;
	delete ground;
	listEntity.clear();
}

void GASimulation::run()
{
	srand(1337);

	//start window
	Window w0( false, "SuperSoup" );
	window0 = &w0;
	//window0->setSize( 1920, 1080 );
	window0->addWindowListener(this);
	window0->addKeyboardListener(this);
	window0->addMouseListener(this);
	window0->create();
	window0->setMaximized();

	//get device context and rendering context
	HDC windowDeviceContext0 = window0->getDeviceContext();
	HGLRC renderingContext0 = wglCreateContext(windowDeviceContext0);

	//this thread must own the gl rendering context or gl calls will be ignored
	BOOL rwglMakeCurrent = wglMakeCurrent(windowDeviceContext0, renderingContext0);

	if(!rwglMakeCurrent)
		throw "wglMakeCurrent failed";

	//setVerticalSync(false);

	glHint(GL_LINE_SMOOTH, GL_NICEST);
	glEnable(GL_LINE_SMOOTH);

	windowResized(window0->getWidth(), window0->getHeight());
	isRunning = true;

	//----------------------------------------------------------------------------------------

	//B2_NOT_USED(argc);
	//B2_NOT_USED(argv);

	Console c;
	c.construct();
	console = &c;

	Thread consoleThread;
	consoleThread.construct(c);
	consoleThread.start();

	PopulationPool plantPopulation;
	PopulationPool animalPopulation;

	plantPopulation.Load("");

	//run X generations
	while(isRunning)
	{
		if(console->isQuit)
			break;

		plantPopulation.SelectTop(10);
		plantPopulation.Grow(20);
		printf("generation %d\n", plantPopulation.generationCounter);

		unsigned int plantNumber = 0;

		for(auto plantIt = plantPopulation.listPopulation.begin(); plantIt != plantPopulation.listPopulation.end(); plantIt++)
		{
			if(console->isQuit)
				break;

			(*plantIt)->Reset();
			(*plantIt)->score = 0;

			if(console->isDebug)
				printf("  plant: %d\n", plantNumber);

			plantNumber++;

			TestPlant(*plantIt);
		}
	}

	printf("render loop end\n");
}
