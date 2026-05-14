#include "SystemsHolder.h"

SystemsHolder* SystemsHolder::instance;

void SystemsHolder::Init(World* world, EntityManager* entMngr, PathFinding* pf)
{
	this->world = world;
	this->entityMananger = entMngr;
	this->pathfinding = pf;
}

void SystemsHolder::Shutdown()
{
	
}