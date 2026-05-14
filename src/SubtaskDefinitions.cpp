#include "SubtaskDefinitions.h"
#include "Worker.h"
#include "PathFinding.h"
#include "SystemsHolder.h"
#include "Commander.h"
#include "Constants.h"
#include "World.h"
#include "EntityManager.h"
#include "Pickup.h"
#include "Building.h"
#include <vector>
#include <iostream>

using namespace SubtaskDefinitions;

ESubtaskState MoveToSubtask::Execute(Worker& worker, float dTime)
{
	// DEBUG teleport
	/*
	worker.position = position;
	worker.target = position;
	return ESubtaskState::Finnished;
	*/

	// Find path
	if (!pathAssigned)
	{
		// Clear previous path
		worker.ClearPath();

		// Find 
		std::map<Node*, NodeRecordAs>* searchResult = SystemsHolder::GetInstance()->commander->searchResult;
		std::priority_queue<NodeRecordAs, std::vector<NodeRecordAs>, NodeRecordAsCompare>* open = SystemsHolder::GetInstance()->commander->open;
		std::vector<Node>* path = SystemsHolder::GetInstance()->pathfinding->AStarDivided(worker.position, position, *searchResult, *open);

		// Continue with path search
		if (path->empty())
			ESubtaskState::Running;

		// Assign path
		std::vector<Vector2> pathPositions;

		for (size_t p = 0; p < path->size(); p++)
		{
			float x = path->at(p).x * GlobalVars::TILE_SIZE;
			float y = path->at(p).y * GlobalVars::TILE_SIZE;
			pathPositions.push_back({ x, y });
		}

		worker.SetNewPath(pathPositions);

		pathAssigned = true;
	}

	// Move to point
	if (worker.PathTargetReached())
		return ESubtaskState::Finnished;

	DrawCircle(worker.target.x, worker.target.y, 10, PINK);
	
	return ESubtaskState::Running;
}

FellTreeSubtask::FellTreeSubtask(TreesTile* tile, float t) :
	treesTile(tile), delay(t) 
{
	ogX = tile->x;
	ogY = tile->y;
}

ESubtaskState FellTreeSubtask::Execute(Worker& worker, float dTime)
{
	if (timer >= delay)
	{
		//Vector2 pos = { treesTile->x * GlobalVars::TILE_SIZE, treesTile->y * GlobalVars::TILE_SIZE };
		Vector2 pos = worker.position; // Hot fix for faulty tile (11,37)

		//std::cout << "feel tree tile: (" << treesTile->x << ", " << treesTile->y << ") - ";
		//std::cout << "pos: (" << pos.x << ", " << pos.y << ") \n";

		// Done
		treesTile->FellTree();
		Pickup* felledTree = new Pickup(Capital::ECapitalType::Tree, pos);
		SystemsHolder::GetInstance()->entityMananger->pickups.push_back(felledTree);

		// Remove empty tree tile
		if (treesTile->amount <= 0)
			SystemsHolder::GetInstance()->world->RemoveTreeTile(treesTile);

		treesTile->reservations--;
		return ESubtaskState::Finnished;
	}

	if (treesTile->y > 30)
		int a = 5;

	// Run 
	timer += dTime;
	return ESubtaskState::Running;
}

ESubtaskState PickupSubtask::Execute(Worker& worker, float dTime)
{
	if (worker.carriedMaterial != Capital::ECapitalType::None)
		return ESubtaskState::Canceled;

	worker.carriedMaterial = pickup->type;
	SystemsHolder::GetInstance()->entityMananger->RemovePickup(pickup);
	return ESubtaskState::Finnished;
}

ESubtaskState PickupFromBuildingSubtask::Execute(Worker& worker, float dTime)
{
	if (worker.carriedMaterial != Capital::ECapitalType::None)
		return ESubtaskState::Canceled;

	worker.carriedMaterial = type;
	building->storedCapital.amounts[type] -= 1;
	building->reservedCapital.amounts[type] -= 1;

	return ESubtaskState::Finnished;
}

ESubtaskState DropItemSubtask::Execute(Worker& worker, float dTime)
{
	if (worker.carriedMaterial == Capital::ECapitalType::None)
		return ESubtaskState::Canceled;

	targetBuilding->storedCapital.amounts[worker.carriedMaterial]++;
	worker.carriedMaterial = Capital::ECapitalType::None;
	return ESubtaskState::Finnished;
}

ESubtaskState TrainWorker::Execute(Worker& worker, float dTime)
{
	if (timer >= delay)
	{
		// Done
		worker.role = role;

		// Change coloring
		switch (role)
		{
		case EWorkerRole::Builder: worker.coloring = YELLOW; break;
		}

		// TODO: Check filter logic
		//EntityManager* entMngr = SystemsHolder::GetInstance()->entityMananger;
		//entMngr->workersRoleFilter[EWorkerRole::General].erase(std::remove(entMngr->workersRoleFilter->begin(), entMngr->workersRoleFilter->end(), &worker));
		//entMngr->workersRoleFilter[role].push_back(&worker);

		worker.trainedRole = EWorkerRole::None;
		return ESubtaskState::Finnished;
	}

	// Run 
	timer += dTime;
	worker.coloring = BLACK;
	return ESubtaskState::Running;
}

ESubtaskState CreateBuilding::Execute(Worker& worker, float dTime)
{
	if (!started)
	{
		building->StartBuilding();
		started = true;
	}

	if (timer >= delay)
	{
		// Done
		building->FinishBuilding();
		return ESubtaskState::Finnished;
	}

	// Run 
	timer += dTime;
	return ESubtaskState::Running;
}

ESubtaskState CreateItem::Execute(Worker& worker, float dTime)
{
	if (!started)
	{
		building->reservedCapital += cost;
		started = true;
	}

	if (timer >= delay)
	{
		// Done
		building->reservedCapital -= cost;
		building->storedCapital -= cost;
		building->storedCapital.amounts[gainItem] += 1;
		return ESubtaskState::Finnished;
	}

	// Run 
	timer += dTime;
	return ESubtaskState::Running;
}

ESubtaskState ScoutSubtask::Execute(Worker& worker, float dTime)
{
	// Find
	if (!foundPath)
	{
		World* world = SystemsHolder::GetInstance()->world;
		PathFinding* pf = SystemsHolder::GetInstance()->pathfinding;

		//target = FindUndiscoveredTile(x, y, &pf, &world);
		foundPath = FindUndiscoveredTile(x, y, *pf, *world);

		return ESubtaskState::Running;
	}

	// Follow
}

bool ScoutSubtask::FindUndiscoveredTile(int x, int y, PathFinding& pf, World& world)
{
	Node* node = &pf.nodes[y][x];

	for (Connection& connection : node->connections)
	{
		Node& cNode = *connection.node;
		/*
		if (world.IsDiscovered(cNode.x, cNode.y))
		{
			//target = Vector2(cNode.x * GlobalVars::TILE_SIZE, cNode.y * GlobalVars::TILE_SIZE);
			//return true;
			return FindUndiscoveredTile(cNode.x, cNode.y, pf, world);
		}
		*/
	}

	/*
	for (Connection& connection : node->connections)
	{
		Node& cNode = *connection.node;
		if (FindUndiscoveredTile(cNode.x, cNode.y, pf, world))
			return true;
	}
	*/

	return false;
}