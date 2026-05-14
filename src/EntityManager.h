#pragma once
#include <vector>
#include "raylib.h"
#include "Capital.h"

enum EWorkerRole;
enum EBuildingType;
class Worker;
class Pickup;
class Building;
class World;

class EntityManager
{
public:
	World* world;
	std::vector<Worker> workers;
	std::vector<Worker*>* workersRoleFilter;

	std::vector<Pickup*> pickups;
	std::vector<Building*> buildings;

	EntityManager();
	~EntityManager();

	void Update(float dTime);

	Worker* FindWorkerOfRole(EWorkerRole role);

	void AddBuilding(Building* building);
	Building* FindBuildingOfType(EBuildingType type);
	Building* FindFinishedBuildingOfType(EBuildingType type);

	void AddPickup(Pickup* pickup);
	void RemovePickup(Pickup* pickup);
	Pickup* FindFreePickupOfType(Capital::ECapitalType type);
	Pickup* FindClosestPickup(Vector2 position, Capital::ECapitalType type);
};