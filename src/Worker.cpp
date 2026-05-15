#include "Worker.h"
#include "SteeringBehavior.h"
#include "Constants.h"
#include "PathFinding.h"
#include "SystemsHolder.h"
#include "EntityManager.h"
#include "World.h"
#include "Pickup.h"
#include "Building.h"
#include "Capital.h"
#include "GoalSystem.h"
#include "SubtaskDefinitions.h"
#include "Database.h"

Worker::Worker(Vector2 starPos)
{
	speed = (1.0 / 10.0) * GlobalVars::TILE_SIZE;
	//speed = 20;
	position = starPos;
	target = position;
	pathNodeDistance = pathNodeDistance * pathNodeDistance;
}

Worker::~Worker()
{

}

void Worker::Update(float dTime)
{
	// Move
	if (path.empty() || FollowPath())
		position = Vector2Add(position, SteeringBehavior::Seek(position, target, speed * dTime));
}

void Worker::Render()
{
	Vector2 pos = position;
	DrawCircle(pos.x, pos.y, Worker::WORKER_SIZE, coloring);
}

void Worker::SetNewPath(std::vector<Vector2>& newPath)
{
	path = newPath;
	currentPathNode = path.size() - 1;
}

bool Worker::FollowPath()
{
	// Path finished?
	if (currentPathNode < 0)
	{
		path.clear();
		currentPathNode = 0;
		return false;
	}

	// Follow next path point
	int x = path[currentPathNode].x;
	int y = path[currentPathNode].y;

	Vector2 pos = { x, y };
	Vector2 dist = Vector2Subtract(pos, position);

	if (Vector2LengthSqr(dist) > pathNodeDistance)
	{
		target = pos;
		//DrawCircle(pos.x, pos.y, 4, ORANGE);
	}
	else
	{
		// Progress to next
		currentPathNode--;
	}

	return true;
}

void Worker::ClearPath()
{
	if (path.empty())
		path.clear();
}

bool Worker::PathTargetReached() 
{
	return currentPathNode < 0;
}

//--------------------------------------------------------------
// Worker task definitions
//--------------------------------------------------------------

namespace WorkerTasks
{
	Task* FellTreeTask(Worker* worker)
	{
		SystemsHolder* systems = SystemsHolder::GetInstance();

		// Find closest tree
		Vector2Int workerPos = World::PositionToTile(worker->position);
		TreesTile* treesTile = systems->world->ClosestTreeTile(workerPos);
		if (!treesTile)
			return nullptr;

		Vector2 treeTilePos = World::TileToCenterPosition({ treesTile->x, treesTile->y });
		float time = GameDB::Database::Instance()->actionCostsResources[GameDB::EActionResource::FellTree].time;

		// Setup subtasks
		Task* task = new Task(worker,
			{
			new SubtaskDefinitions::MoveToSubtask(treeTilePos),
			new SubtaskDefinitions::FellTreeSubtask(treesTile, time)
			}
		);

		task->name = "FellTree";
		treesTile->reservations++;

		// Input - Output
		//task->output = TaskAttribute(ETaskAttributeCategory::Capital, Capital::ECapitalType::Tree, 1, nullptr);

		return task;
	}

	Task* DeliverItemTask(Worker* worker, Capital::ECapitalType itemType, Building* target)
	{
		if (itemType == Capital::ECapitalType::IronOre)
			int a = 5;

		SystemsHolder* systems = SystemsHolder::GetInstance();

		if (itemType == Capital::ECapitalType::Coal)
			int a = 5;

		// Find closest item
		Pickup* item = systems->entityMananger->FindClosestPickup(worker->position, itemType);
		if (!item || !target)
		{
			//throw std::runtime_error("World out of resources!");
			return nullptr;
		}

		// Setup subtasks
		Task* task = new Task(worker,
			{
			new SubtaskDefinitions::MoveToSubtask(item->position),
			new SubtaskDefinitions::PickupSubtask(item),
			new SubtaskDefinitions::MoveToSubtask(target->position),
			new SubtaskDefinitions::DropItemSubtask(target)
			}
		);

		task->name = "W Deliver " + std::to_string(itemType);
		item->reserved = true;
		// Input - Output
		/*
		TaskAttribute input = TaskAttribute(ETaskAttributeCategory::Capital, itemType, 1, nullptr);
		task->input.push_back(input);

		task->output = TaskAttribute(ETaskAttributeCategory::Capital, itemType, 1, target);
		*/

		return task;
	}

	Task* DeliverFromBuildingTask(Worker* worker, Capital::ECapitalType itemType, Building* from, Building* target)
	{
		if (!target)
			return nullptr;

		SystemsHolder* systems = SystemsHolder::GetInstance();

		// Check availability
		int itemAmount = from->GetAvailableCapital()[itemType];
		int actual = from->storedCapital[itemType];
		int reserved = from->reservedCapital[itemType];

		if (itemAmount <= 0)
		{
			std::string err = "Building: " + std::to_string(from->type) + " out of resource: " + std::to_string(itemType);
			throw std::runtime_error(err);
			return nullptr;
		}

		// Reserve capital
		if (itemType == Capital::ECapitalType::Coal)
			int a = 4;

		from->reservedCapital.amounts[itemType] += 1;

		// Setup subtasks
		Task* task = new Task(worker,
			{
			new SubtaskDefinitions::MoveToSubtask(from->position),
			new SubtaskDefinitions::PickupFromBuildingSubtask(from, itemType),
			new SubtaskDefinitions::MoveToSubtask(target->position),
			new SubtaskDefinitions::DropItemSubtask(target)
			}
		);

		task->name = "Deliver " + std::to_string(itemType);

		// Input - Output
		/*
		TaskAttribute input = TaskAttribute(ETaskAttributeCategory::Capital, itemType, 1, from);
		task->input.push_back(input);

		task->output = TaskAttribute(ETaskAttributeCategory::Capital, itemType, 1, target);
		*/

		return task;
	}

	Task* TrainForRoleTask(Worker* worker, EWorkerRole role)
	{
		// Find training time
		GameDB::EActionTraining trainingType = GameDB::EActionTraining::TrainScout;
		switch (role)
		{
			case EWorkerRole::Scout: trainingType = GameDB::EActionTraining::TrainScout; break;
			case EWorkerRole::Builder: trainingType = GameDB::EActionTraining::TrainBuilder; break;
			case EWorkerRole::Soldier: trainingType = GameDB::EActionTraining::TrainSoldier; break;
			default: trainingType = GameDB::EActionTraining::TrainCraftsman; break;
		}

		float time = GameDB::Database::Instance()->actionCostsTraining[trainingType].time;

		// Setup task
		Task* task = new Task(nullptr,
			{
			new SubtaskDefinitions::TrainWorker(role, time)
			}
		);

		task->name = "Train " + std::to_string(role);

		// Input - Output
		/*
		task->output.category = ETaskAttributeCategory::Worker;
		task->output.type = role;
		task->output.amount = 1;
		*/

		return task;
	}

	Task* TrainForSoldierTask(Worker* worker, Building* trainingCamp)
	{
		float time = GameDB::Database::Instance()->actionCostsTraining[GameDB::EActionTraining::TrainSoldier].time;

		// Setup task
		Task* task = new Task(nullptr,
			{
			new SubtaskDefinitions::MoveToSubtask(trainingCamp->position),
			new SubtaskDefinitions::TrainWorker(EWorkerRole::Soldier, time)
			}
		);

		task->name = "Train soldier";

		return task;
	}

	Task* BuildTask(Worker* worker, Building* building)
	{
		if (!building)
			return nullptr;

		// Building task
		float time = GameDB::Database::Instance()->actionCostsBuilding[building->type].time;
		building->state = EBuildingState::InProgress;

		// Setup task
		Task* task = new Task(worker,
			{
			new SubtaskDefinitions::MoveToSubtask(building->position),
			new SubtaskDefinitions::CreateBuilding(building, time)
			}
		);

		task->name = "Build " + std::to_string(building->type);

		// Input - Output
		/*
		TaskAttribute input = TaskAttribute(ETaskAttributeCategory::Worker, EWorkerRole::Builder, 1, nullptr);
		task->input.push_back(input);

		task->output = TaskAttribute(ETaskAttributeCategory::Building, building->type, 1, nullptr);
		*/

		return task;
	}

	Task* CreateItemTask(Worker* worker, Building* building, Capital::ActionCost cost, Capital::ECapitalType gainItem)
	{
		// Building task
		float time = GameDB::Database::Instance()->actionCostsBuilding[building->type].time;

		// Setup task
		Task* task = new Task(worker,
			{
			new SubtaskDefinitions::MoveToSubtask(building->position),
			new SubtaskDefinitions::CreateItem(building, cost.time, cost.capital, gainItem)
			//new SubtaskDefinitions::CreateBuilding(building, time)
			}
		);

		task->name = "Cr coal";

		return task;
	}

	Task* ScoutTask(Worker* worker)
	{
		// Find
		//int x = worker->position.x / GlobalVars::TILE_SIZE;
		//int y = worker->position.y / GlobalVars::TILE_SIZE;
		//SubtaskDefinitions::ScoutSubtask scout = SubtaskDefinitions::ScoutSubtask(x, y);
		//scout.Execute(*worker, 0);

		Node* node = SystemsHolder::GetInstance()->pathfinding->ClosestUndiscovered(worker->position);
		if (!node)
			return nullptr;

		Vector2 pos = { node->x * GlobalVars::TILE_SIZE, node->y * GlobalVars::TILE_SIZE };

		// Setup task
		Task* task = new Task(worker,
			{
			new SubtaskDefinitions::MoveToSubtask(pos)
			//new SubtaskDefinitions::CreateItem(building, cost.time, cost.capital, gainItem)
			//new SubtaskDefinitions::CreateBuilding(building, time)
			}
		);

		task->name = "Scout";

		return task;
	}
}
