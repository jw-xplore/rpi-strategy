#pragma once

class World;
class EntityManager;
class PathFinding;
class Commander;

class SystemsHolder
{
private:
	static SystemsHolder* instance;

public:
	World* world;
	EntityManager* entityMananger;
	PathFinding* pathfinding;
	Commander* commander;

	SystemsHolder() {}
	~SystemsHolder() {}

	static SystemsHolder* GetInstance()
	{
		if (!instance)
			instance = new SystemsHolder();

		return instance;
	}

	void Init(World* world, EntityManager* entMngr, PathFinding* pf);
	void Shutdown();
};

