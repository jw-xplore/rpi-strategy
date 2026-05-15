#include "EntityManager.h"
#include "Database.h"
#include "Worker.h"
#include "Constants.h"
#include "SteeringBehavior.h"
#include "Pickup.h"
#include "Capital.h"
#include "Building.h"
#include "World.h"
#include "SystemsHolder.h"
#include "Commander.h"

EntityManager::EntityManager()
{
	GameDB::Database* db = GameDB::Database::Instance();

	int startTileX = GlobalVars::START_TILE_X;
	int startTileY = GlobalVars::START_TILE_Y;
	int endTileX = GlobalVars::START_TILE_X + 3;
	int	endTileY = GlobalVars::START_TILE_Y + 3;

	Vector2 startX = { startTileX * GlobalVars::TILE_SIZE, endTileX * GlobalVars::TILE_SIZE };
	Vector2 startY = { startTileY * GlobalVars::TILE_SIZE, endTileY * GlobalVars::TILE_SIZE };

	// Setup worker role filter
	for (size_t i = 0; i < EWorkerRole::EWorkerRoleCount; i++)
	{
		workersRoleFilter = new std::vector<Worker*>[EWorkerRole::EWorkerRoleCount];
	}

	// Setup workers
	int population = db->startingPopulation;
	//population = 8;

	//workers = new std::vector<Worker>();
	workers.reserve(population);

	for (size_t i = 0; i < population; i++)
	{
		Vector2 randPos = { GetRandomValue(startX.x, startX.y), GetRandomValue(startY.x, startY.y) };
		Worker worker = Worker(randPos);
		worker.id = i;
		workers.push_back(worker);

		//workersRoleFilter[EWorkerRole::General].push_back(&worker);
	}

	// Setup pickups
	int ironAmount = db->ironOreAmount;
	//ironAmount = 100000;

	pickups.reserve(ironAmount * 2);

	for (size_t i = 0; i < ironAmount; i++)
	{
		Vector2 randPos = { GetRandomValue(24, 100 * GlobalVars::TILE_SIZE - 24), GetRandomValue(24, 100 * GlobalVars::TILE_SIZE - 24) };
		pickups.push_back(new Pickup(Capital::ECapitalType::IronOre, randPos));
	}

	// Setup buildings
	buildings.reserve(10);
}

EntityManager::~EntityManager()
{
	workers.clear();
	//delete workers;

	pickups.clear();
	//delete pickups;
}

void EntityManager::Init()
{
	Image workerImg = LoadImage("resources/worker.png");
	workerTexture = LoadTextureFromImage(workerImg);
	UnloadImage(workerImg);
}

void EntityManager::Update(float dTime)
{
	Commander* commander = SystemsHolder::GetInstance()->commander;

	// Workers
	for (Worker& worker : workers)
	{
		worker.Update(dTime);
		//workers.at(i).Render();

		DrawTexture(workerTexture, worker.position.x, worker.position.y, worker.coloring);

		// Debug
		Color col = BLACK;
		if (commander->activeTasks[worker.id])
			col = YELLOW;

		DrawText(std::to_string(worker.id).c_str(), worker.position.x, worker.position.y, 4, col);
	}

	// Render pickups
	for (auto& pickup : pickups)
	{
		DrawRectangle(pickup->position.x, pickup->position.y, pickup->size.x, pickup->size.y, pickup->color);
	}

	// Render buildings
	for (auto& building : buildings)
	{
		DrawRectangle(building->position.x, building->position.y, GlobalVars::TILE_HALF_SIZE, GlobalVars::TILE_HALF_SIZE, building->color);
	
		// Debug
		int* available = building->GetAvailableCapital().amounts;
		int* reserved = building->reservedCapital.amounts;

		std::string buildStr = "w: " + std::to_string(available[Capital::ECapitalType::Tree]) + " (" + std::to_string(reserved[Capital::ECapitalType::Tree]) + ")";
		buildStr += ", co: " + std::to_string(available[Capital::ECapitalType::Coal]) + " (" + std::to_string(reserved[Capital::ECapitalType::Coal]) + ")";
		buildStr += ", io: " + std::to_string(available[Capital::ECapitalType::IronOre]) + " (" + std::to_string(reserved[Capital::ECapitalType::IronOre]) + ")";
		buildStr += ", ib: " + std::to_string(available[Capital::ECapitalType::IronBar]) + " (" + std::to_string(reserved[Capital::ECapitalType::IronBar]) + ")";
		
		DrawText(buildStr.c_str(), building->position.x, building->position.y, 8, BLACK);
	}
}

Worker* EntityManager::FindWorkerOfRole(EWorkerRole role)
{
	for (Worker& worker : workers)
	{
		if (worker.role == role)
			return &worker;
	}

	return nullptr;
}

Building* EntityManager::FindBuildingOfType(EBuildingType type)
{
	for (Building*& building : buildings)
	{
		// Ignore undiscovered
		//if (world->TileDiscoveryState(building->position.x / GlobalVars::TILE_SIZE, building->position.y / GlobalVars::TILE_SIZE) != EDiscovetyState::Discovered)
			//continue;

		if (building->type == type)
			return building;
	}

	return nullptr;
}

Building* EntityManager::FindFinishedBuildingOfType(EBuildingType type)
{
	for (Building*& building : buildings)
	{
		// Ignore undiscovered
		//if (world->TileDiscoveryState(building->position.x / GlobalVars::TILE_SIZE, building->position.y / GlobalVars::TILE_SIZE) != EDiscovetyState::Discovered)
			//continue;

		if (building->type == type && building->state == EBuildingState::Finished)
			return building;
	}

	return nullptr;
}

void EntityManager::AddPickup(Pickup* pickup)
{
	pickups.push_back(pickup);
}

void EntityManager::RemovePickup(Pickup* pickup)
{
	//pickups.erase(find(pickups.begin(), pickups.end(), *pickup));
	pickups.erase(std::remove(pickups.begin(), pickups.end(), pickup), pickups.end());
}

Pickup* EntityManager::FindFreePickupOfType(Capital::ECapitalType type)
{
	for (Pickup*& pickup : pickups)
	{
		if (pickup->type == type && !pickup->reserved)
		{
			// Ignore undiscovered
			if (world->TileDiscoveryState(pickup->position.x / GlobalVars::TILE_SIZE, pickup->position.y / GlobalVars::TILE_SIZE) != EDiscovetyState::Discovered)
				continue;

			return pickup;
		}
	}

	return nullptr;
}

Pickup* EntityManager::FindClosestPickup(Vector2 position, Capital::ECapitalType type)
{
	Pickup* closestPickup = nullptr;
	float closest = -1;

	// TODO: Find closest pickup in relation to position and target (e.g. closest for Worker and target Building)
	for (Pickup*& pickup : pickups)
	{
		if (pickup->type != type)
			continue;

		if (pickup->reserved)
			continue;

		// Ignore undiscovered 
		if (world->TileDiscoveryState(pickup->position.x / GlobalVars::TILE_SIZE, pickup->position.y / GlobalVars::TILE_SIZE) != EDiscovetyState::Discovered)
			continue;

		Vector2 diff = { pickup->position.x - position.x, pickup->position.y - position.y };

		float dist = diff.x * diff.x + diff.y * diff.y;

		if (closest == -1 || closest > dist)
		{
			closestPickup = pickup;
			closest = dist;
		}
	}

	return closestPickup;
}

void EntityManager::AddBuilding(Building* building)
{
	buildings.push_back(building);
}