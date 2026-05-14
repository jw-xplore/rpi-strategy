#pragma once
#include "Capital.h"
#include <vector>
#include "raylib.h"
#include "types.h"

class PathFinding;
class Pickup;
class Building;
struct TreesTile;
class Task;

/*
enum EWorkerRole
{
	None = -1,
	General,
	Scout,
	CoalMiner,
	ArmsSmith,
	SmelterOperator,
	Builder,
	Soldier,
	EWorkerRoleCount
};
*/

class Worker
{
private:
	std::vector<Vector2> path = {};
	int currentPathNode = 0;

public:
	int id = -1;
	const int WORKER_SIZE = 2;
	const Color WORKER_COLOR = RED;

	EWorkerRole role = EWorkerRole::General;
	EWorkerRole trainedRole = EWorkerRole::None;
	Capital::ECapitalType carriedMaterial = Capital::ECapitalType::None;
	Color coloring = WORKER_COLOR;

	// Transform
	Vector2 position;
	Vector2 target;
	float speed;

	// Path
	float pathNodeDistance = 10;

	Worker(){}
	Worker(Vector2 starPos);
	~Worker();

	void Update(float dTime);
	void Render();

	void SetNewPath(std::vector<Vector2>& newPath);
	bool FollowPath();
	void ClearPath();
	bool PathTargetReached();
};

namespace WorkerTasks
{
	Task* FellTreeTask(Worker* worker);
	Task* DeliverItemTask(Worker* worker, Capital::ECapitalType itemType, Building* target);
	Task* DeliverFromBuildingTask(Worker * worker, Capital::ECapitalType itemType, Building * from, Building * target);
	Task* TrainForRoleTask(Worker* worker, EWorkerRole role);
	Task* TrainForSoldierTask(Worker* worker, Building* trainingCamp);
	Task* BuildTask(Worker* worker, Building* building);
	Task* CreateItemTask(Worker* worker, Building* building, Capital::ActionCost cost, Capital::ECapitalType gainItem);
	Task* ScoutTask(Worker* worker);
};