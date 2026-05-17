#include "Commander.h"
#include "Worker.h"
#include "Building.h"
#include "SystemsHolder.h"
#include "EntityManager.h"
#include "GoalSystem.h"
#include "SubtaskDefinitions.h"
#include "World.h"
#include "Constants.h"
#include "PathFinding.h"
#include "Database.h"
#include <iostream>

Commander::Commander()
{
	searchResult = new std::map<Node*, NodeRecordAs>();
	open = new std::priority_queue<NodeRecordAs, std::vector<NodeRecordAs>, NodeRecordAsCompare>();

	SystemsHolder* systemsHolder = SystemsHolder::GetInstance();
	systemsHolder->commander = this;
	entityManager = systemsHolder->entityMananger;

	world = systemsHolder->world;
	pathfinding = systemsHolder->pathfinding;

	// Preplace buildings 
	entityManager->AddBuilding(new Building(EBuildingType::CoalMile, {160, 160}));
	entityManager->AddBuilding(new Building(EBuildingType::Smelter, { 120, 80 }));
	entityManager->AddBuilding(new Building(EBuildingType::ArsmithsForge, { 160, 100 }));
	entityManager->AddBuilding(new Building(EBuildingType::TrainingCamp, { 120, 180 }));

	DefineAvailableTasks();

	int population = entityManager->workers.size();

	// Setup active tasks
	activeTasks.reserve(population);
	for (int ti = 0; ti < population; ti++)
	{
		activeTasks.push_back(nullptr);
		//activeTasks[ti] = nullptr;
	}

	// Scouts setup
	scoutsPos = population - dedicatedScouts;

	for (int i = scoutsPos; i < population; i++)
	{
		Worker* worker = &entityManager->workers.at(i);
		scouts.push_back(worker);
		AssignTask(worker, WorkerTasks::TrainForRoleTask(worker, EWorkerRole::Scout));
	}

	/*
	scouts[0]->position = Vector2(420, 300);
	scouts[0]->target = Vector2(420, 300);
	*/
}

Commander::~Commander()
{

}

void Commander::DefineAvailableTasks()
{
	GameDB::Database* db = GameDB::Database::Instance();
	//db->actionCostsResources[EActionResource::MakeCoal];

	// Buildings
	Building* coalMile = entityManager->FindBuildingOfType(EBuildingType::CoalMile);
	Building* smelter = entityManager->FindBuildingOfType(EBuildingType::Smelter);
	Building* forge = entityManager->FindBuildingOfType(EBuildingType::ArsmithsForge);
	Building* camp = entityManager->FindBuildingOfType(EBuildingType::TrainingCamp);

	// Material gathering
	GoalStep* cutWood = new GoalStep(
		"Fell tree",
		{},
		TaskAttribute(ETaskAttributeCategory::Capital, Capital::ECapitalType::Tree, 1, nullptr),
		[*this](Worker* worker) { return  WorkerTasks::FellTreeTask(worker); }
	);

	// Creation
	int* createCoalAmounts = db->actionCostsResources[GameDB::EActionResource::MakeCoal].capital.amounts;

	GoalStep* createCoal = new GoalStep(
		"Cr coal",
		{
			TaskAttribute(ETaskAttributeCategory::Building, EBuildingType::CoalMile, 1, nullptr, true),
			TaskAttribute(ETaskAttributeCategory::Capital, Capital::ECapitalType::Tree, createCoalAmounts[Capital::ECapitalType::Tree], coalMile),
			TaskAttribute(ETaskAttributeCategory::Worker, EWorkerRole::CoalMiner, 1, nullptr)
		},
		TaskAttribute(ETaskAttributeCategory::Capital, Capital::ECapitalType::Coal, 1, coalMile),
		[*this, db](Worker* worker) { return  WorkerTasks::CreateItemTask(
			worker,
			entityManager->FindBuildingOfType(EBuildingType::CoalMile), 
			db->actionCostsResources[GameDB::EActionResource::MakeCoal],
			Capital::ECapitalType::Coal
		); 
		}
	);

	createCoal->roleConstrain = EWorkerRole::CoalMiner;

	int* createIronBarAmounts = db->actionCostsResources[GameDB::EActionResource::MakeIronBar].capital.amounts;

	GoalStep* createIronBar = new GoalStep(
		"Cr ironB",
		{
			TaskAttribute(ETaskAttributeCategory::Building, EBuildingType::Smelter, 1, nullptr, true),
			TaskAttribute(ETaskAttributeCategory::Capital, Capital::ECapitalType::Coal, createIronBarAmounts[Capital::ECapitalType::Coal], smelter),
			TaskAttribute(ETaskAttributeCategory::Capital, Capital::ECapitalType::IronOre, createIronBarAmounts[Capital::ECapitalType::IronOre], smelter),
			TaskAttribute(ETaskAttributeCategory::Worker, EWorkerRole::SmelterOperator, 1, nullptr)
		},
		TaskAttribute(ETaskAttributeCategory::Capital, Capital::ECapitalType::IronBar, 1, smelter),
		[*this, db](Worker* worker) { return  WorkerTasks::CreateItemTask(
			worker,
			entityManager->FindBuildingOfType(EBuildingType::Smelter),
			db->actionCostsResources[GameDB::EActionResource::MakeIronBar],
			Capital::ECapitalType::IronBar
		);
		}
	);

	createIronBar->roleConstrain = EWorkerRole::SmelterOperator;

	int* createSwordAmounts = db->actionCostsResources[GameDB::EActionResource::MakeSword].capital.amounts;

	GoalStep* createSword = new GoalStep(
		"Cr sword",
		{
			TaskAttribute(ETaskAttributeCategory::Building, EBuildingType::ArsmithsForge, 1, nullptr, true),
			TaskAttribute(ETaskAttributeCategory::Capital, Capital::ECapitalType::Coal, createSwordAmounts[Capital::ECapitalType::Coal], forge),
			TaskAttribute(ETaskAttributeCategory::Capital, Capital::ECapitalType::IronBar, createSwordAmounts[Capital::ECapitalType::IronBar], forge),
			TaskAttribute(ETaskAttributeCategory::Worker, EWorkerRole::ArmsSmith, 1, nullptr)
		},
		TaskAttribute(ETaskAttributeCategory::Capital, Capital::ECapitalType::Sword, 1, forge),
		[*this, db](Worker* worker) { return  WorkerTasks::CreateItemTask(
			worker,
			entityManager->FindBuildingOfType(EBuildingType::ArsmithsForge),
			db->actionCostsResources[GameDB::EActionResource::MakeSword],
			Capital::ECapitalType::Sword
		);
		}
	);

	createSword->roleConstrain = EWorkerRole::ArmsSmith;

	availableSteps.push_back(cutWood);
	availableSteps.push_back(createCoal);
	availableSteps.push_back(createIronBar);
	availableSteps.push_back(createSword);

	/*
	availableSteps.push_back(new GoalStep(
		"Deliver wood",
		{
			TaskAttribute(ETaskAttributeCategory::Capital, Capital::ECapitalType::Tree, 1, nullptr)
		},
		TaskAttribute(ETaskAttributeCategory::Worker, EWorkerRole::Scout, 1, nullptr)
	));
	*/

	//--------------

	// Goals final tasks
	for (size_t i = 0; i < EWorkerRole::EWorkerRoleCount; i++)
	{
		// Soldier will have custon task
		if (i == EWorkerRole::Soldier)
			continue;

		std::string name = "";
		switch (i)
		{
		case EWorkerRole::Scout: name = "Scout"; break;
		case EWorkerRole::Builder: name = "Builder"; break;
		case EWorkerRole::CoalMiner: name = "CoalM"; break;
		case EWorkerRole::SmelterOperator: name = "Smelt"; break;
		case EWorkerRole::ArmsSmith: name = "Smith"; break;
		}

		GoalStep* train = new GoalStep(
			"Train " + name,
			{},
			TaskAttribute(ETaskAttributeCategory::Worker, static_cast<EWorkerRole>(i), 1, nullptr),
			[*this, i](Worker* worker) { return WorkerTasks::TrainForRoleTask(worker, static_cast<EWorkerRole>(i)); }
		);

		availableSteps.push_back(train);
	}

	// Train soldier
	GoalStep* trainSoldier = new GoalStep(
		"Train soldier",
		{
			TaskAttribute(ETaskAttributeCategory::Building, EBuildingType::TrainingCamp, 1, nullptr, true),
			TaskAttribute(ETaskAttributeCategory::Capital, Capital::ECapitalType::Sword, 1, entityManager->FindBuildingOfType(EBuildingType::TrainingCamp)),
		},
		TaskAttribute(ETaskAttributeCategory::Worker, EWorkerRole::Soldier, 1, nullptr),
		[*this](Worker* worker) { return WorkerTasks::TrainForSoldierTask(worker, entityManager->FindBuildingOfType(EBuildingType::TrainingCamp)); }
	);

	trainSoldier->totalTasks = 20;

	availableSteps.push_back(trainSoldier);

	// Define delivery tasks
	GoalStep* deliverItem = new GoalStep(
		"Deliver",
		{
			//TaskAttribute(ETaskAttributeCategory::Capital, Capital::ECapitalType::Tree, 1, nullptr)
		},
		TaskAttribute(ETaskAttributeCategory::Capital, -1, 0, nullptr),
		nullptr
		/*
		[*this](Worker* worker) {
			return WorkerTasks::DeliverItemTask(worker, Capital::ECapitalType::Tree, nullptr);
		}
		*/
	);

	deliverItem->variableInOut = true;
	deliverItem->isDelivery = true;

	availableSteps.push_back(deliverItem);

	// Buidling tasks 
	GoalStep* buildCoalMile = new GoalStep(
		"Build coal mile",
		{
			TaskAttribute(ETaskAttributeCategory::Capital, Capital::ECapitalType::Tree, 10, entityManager->FindBuildingOfType(EBuildingType::CoalMile)),
			TaskAttribute(ETaskAttributeCategory::Worker, EWorkerRole::Builder, 1, nullptr)
		},
		TaskAttribute(ETaskAttributeCategory::Building, EBuildingType::CoalMile, 1, nullptr),
		[*this](Worker* worker) { return WorkerTasks::BuildTask(worker, entityManager->FindBuildingOfType(EBuildingType::CoalMile)); }
	);

	buildCoalMile->roleConstrain = EWorkerRole::Builder;
	buildCoalMile->doneEvaluateReality = true;

	GoalStep* buildSmelter = new GoalStep(
		"Build smelter",
		{
			TaskAttribute(ETaskAttributeCategory::Capital, Capital::ECapitalType::Tree, 10, entityManager->FindBuildingOfType(EBuildingType::Smelter)),
			TaskAttribute(ETaskAttributeCategory::Worker, EWorkerRole::Builder, 1, nullptr)
		},
		TaskAttribute(ETaskAttributeCategory::Building, EBuildingType::Smelter, 1, nullptr),
		[*this](Worker* worker) { return WorkerTasks::BuildTask(worker, entityManager->FindBuildingOfType(EBuildingType::Smelter)); }
	);

	buildCoalMile->roleConstrain = EWorkerRole::Builder;
	buildSmelter->doneEvaluateReality = true;

	GoalStep* buildForge = new GoalStep(
		"Build forge",
		{
			TaskAttribute(ETaskAttributeCategory::Capital, Capital::ECapitalType::Tree, 10, entityManager->FindBuildingOfType(EBuildingType::ArsmithsForge)),
			TaskAttribute(ETaskAttributeCategory::Capital, Capital::ECapitalType::IronBar, 3, entityManager->FindBuildingOfType(EBuildingType::ArsmithsForge)),
			TaskAttribute(ETaskAttributeCategory::Worker, EWorkerRole::Builder, 1, nullptr)
		},
		TaskAttribute(ETaskAttributeCategory::Building, EBuildingType::ArsmithsForge, 1, nullptr),
		[*this](Worker* worker) { return WorkerTasks::BuildTask(worker, entityManager->FindBuildingOfType(EBuildingType::ArsmithsForge)); }
	);

	buildForge->roleConstrain = EWorkerRole::Builder;
	buildForge->doneEvaluateReality = true;

	GoalStep* buildCamp = new GoalStep(
		"Build camp",
		{
			TaskAttribute(ETaskAttributeCategory::Capital, Capital::ECapitalType::Tree, 10, entityManager->FindBuildingOfType(EBuildingType::TrainingCamp)),
			TaskAttribute(ETaskAttributeCategory::Worker, EWorkerRole::Builder, 1, nullptr)
		},
		TaskAttribute(ETaskAttributeCategory::Building, EBuildingType::TrainingCamp, 1, nullptr),
		[*this](Worker* worker) { return WorkerTasks::BuildTask(worker, entityManager->FindBuildingOfType(EBuildingType::TrainingCamp)); }
	);

	buildCamp->roleConstrain = EWorkerRole::Builder;
	buildCamp->doneEvaluateReality = true;

	availableSteps.push_back(buildCoalMile);
	availableSteps.push_back(buildSmelter);

	// Goals definition
	goals.push_back(new Goal(*buildCoalMile, availableSteps));
	goals.push_back(new Goal(*buildSmelter, availableSteps));
	//return;
	goals.push_back(new Goal(*buildForge, availableSteps));
	goals.push_back(new Goal(*buildCamp, availableSteps));
	goals.push_back(new Goal(*trainSoldier, availableSteps));
}

void Commander::Update(float dTime)
{
	// Assigne new tasks
	replanTimer += dTime;

	/*
	if (replanTimer >= replanDelay)
	{
		UpdatePlan();
		replanTimer = 0;
	}
	*/

	UpdatePlan();

	// Update tasks
	int sz = activeTasks.size();

	for (int i = 0; i < sz; i++)
	{
		Task* wTask = activeTasks[i];

		if (!wTask)
			continue;

		if (wTask->finished)
		{
			// Finished - Remove
			//*goals[currentGoal]->potentialCapital -= wTask.second->rewardCapital;

			if (wTask->parentGoalStep != nullptr)
				wTask->parentGoalStep->finishedTasks++;

			activeTasks[i] = nullptr;
			continue;
		}

		wTask->Update(dTime);
	}

	// Scouting update
	for (Worker*& scout : scouts)
	{
		ScoutPos(scout->position.x, scout->position.y);
	}
}

void Commander::UpdatePlan()
{
	//int i = 0;

	/*
	for (Worker& worker : entityManager->workers)
	{
		if (activeTasks[i])
			continue;

		for (Goal*& goal : goals)
		{
			GoalStep* step = goal->NextAvailableStep();
			if (step)
			{
				step->AssignTask();
				break;
			}
			else
			{
				goal->NextAvailableStep();
			}
		}

		i++;
		if (i >= scoutsPos)
			break;
	}

	// Scouts
	int end = dedicatedScouts + scoutsPos;
	for (int i = scoutsPos; i < end; i++)
	{
		Task* active = activeTasks[i];
		if (activeTasks[i] != nullptr)
			continue;

		int si = i - scoutsPos;
		AssignTask(scouts[si], WorkerTasks::ScoutTask(scouts[si]));
	}
	*/

	int size = entityManager->workers.size();
	int limit = 10;
	int checks = 0;

	// Move to next if work is taken
	while (activeTasks[workerToPlan])
	{
		workerToPlan++;
		if (workerToPlan >= size)
			workerToPlan = 0;

		checks++;
		if (checks > limit)
			return;
	}

	// Update selected worker plan
	if (workerToPlan < scoutsPos)
	{
		// Workers
		for (Goal*& goal : goals)
		{
			GoalStep* step = goal->NextAvailableStep();
			if (step)
			{
				step->AssignTask(&entityManager->workers[workerToPlan]);
				break;
			}
			else
			{
				// Debug
				goal->NextAvailableStep();
			}
		}
	}
	else
	{
		// Scouts
		if (!pathfinding->nextUndiscovered.empty())
		{
			int si = workerToPlan - scoutsPos;
			AssignTask(scouts[si], WorkerTasks::ScoutTask(scouts[si]));
		}
	}

	// Move to next
	workerToPlan++;
	if (workerToPlan >= size)
		workerToPlan = 0;
}

int displayTask = 0;

void Commander::DebugDraw()
{
	if (IsKeyDown(KEY_ONE))
		displayTask = 0;
	if (IsKeyDown(KEY_TWO))
		displayTask = 1;
	if (IsKeyDown(KEY_THREE))
		displayTask = 2;
	if (IsKeyDown(KEY_FOUR))
		displayTask = 3;
	if (IsKeyDown(KEY_FIVE))
		displayTask = 4;
		

	if (goals[displayTask]->finalStep)
		goals[displayTask]->DebugDraw(*goals[displayTask]->finalStep, 0, 0);

	// Worker stats
	std::string workerStats = "";
	int working = 0;

	int i = 0;
	for (auto& worker : entityManager->workers)
	{
		std::string taskName = "";

		if (activeTasks[i])
		{
			taskName = activeTasks[i]->name;
			working++;
		}

		std::string soldierStr = "";
		if (worker.role == EWorkerRole::Soldier)
			soldierStr = "[SOLDIER]";

		workerStats += soldierStr + std::to_string(worker.id) + ". (" + std::to_string(worker.role) + ") task: " + taskName + " (car: " + std::to_string(worker.carriedMaterial) + ")\n";
	
		i++;
	}

	DrawText(("Working: " + std::to_string(working)).c_str(), -20, 100, 10, YELLOW);
	DrawText(workerStats.c_str(), -20, 120, 10, YELLOW);
}

void Commander::ScoutPos(float posX, float posY)
{
	int x = posX / GlobalVars::TILE_SIZE;
	int y = posY / GlobalVars::TILE_SIZE;

	if (x < 1 || x >= world->worldSize)
		return;

	if (y < 1 || y >= world->worldSize)
		return;

	//if (world->TileDiscoveryState(x, y) == EDiscovetyState::Discovered)
		//return;

	/*
	world->discovered[x][y] = true;
	pathfinding->Discover(x, y);

	world->discovered[x + 1][y] = true;
	pathfinding->Discover(x + 1, y);

	world->discovered[x + 1][y + 1] = true;
	pathfinding->Discover(x + 1, y + 1);

	world->discovered[x - 1][y] = true;
	pathfinding->Discover(x - 1, y);
	*/

	pathfinding->Discover(x, y);
	pathfinding->Discover(x, y + 1);
	pathfinding->Discover(x + 1, y + 1);
	pathfinding->Discover(x + 1, y);
	pathfinding->Discover(x + 1, y - 1);
	pathfinding->Discover(x, y - 1);
	pathfinding->Discover(x - 1, y - 1);
	pathfinding->Discover(x - 1, y);
	pathfinding->Discover(x - 1, y + 1);
}

Worker* Commander::FindFreeWorker(EWorkerRole roleConstrain)
{
	// Find worker of exactly matching role
	int i = 0;

	for (Worker& worker : entityManager->workers)
	{
		// Get free worker
		if (activeTasks[worker.id] == nullptr)
		{
			if (worker.role != roleConstrain)
				continue;

			return &worker;
		}

		i++;
		if (i >= scoutsPos)
			break;
	}

	/*
	// Consider non-general worker for general task
	if (roleConstrain == EWorkerRole::General)
	{
		i = 0;

		for (Worker& worker : entityManager->workers)
		{
			// Get free worker
			if (activeTasks[i] == nullptr)
			{
				if (roleConstrain != EWorkerRole::General && worker.role != roleConstrain)
					continue;

				return &worker;
			}

			i++;
			if (i >= scoutsPos)
				break;
		}
	}
	*/

	return nullptr;
}

void Commander::AssignTask(Worker* worker, Task* task)
{
	if (activeTasks[worker->id])
		int a = 4;

	activeTasks[worker->id] = task;
	task->assignee = worker;

	//*goals[currentGoal]->potentialCapital += task->rewardCapital;
}