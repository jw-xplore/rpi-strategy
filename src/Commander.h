#pragma once
/*
Takes care of high-level decision making and assigning tasks
*/

#include <vector>
#include <queue>
#include <map>
#include <algorithm>
#include "Capital.h"

class Worker;
enum EWorkerRole;
class Task;
class Goal;
class GoalStep;
enum EBuildingType;

struct Node;
struct NodeRecordAs;
struct NodeRecordAsCompare;
class PathFinding;
class World;

class EntityManager;
class DecisionTreeNode;
class Decision;

class Commander;
class Building;

//--------------------------------------------------------------
// Commander
//--------------------------------------------------------------

struct CommnaderStateTracking
{
	// Tracks game state from commnader perspective
	int scoutsAmount;
	
	// Buildings
	Building* coalMile;
	Building* arsmithsForge;
	Building* smelter;
	Building* trainingCamp;
};

class Commander
{
public:
	CommnaderStateTracking stateTracking;
	EntityManager* entityManager;
	World* world;
	PathFinding* pathfinding;

	// Path finding
	std::map<Node*, NodeRecordAs>* searchResult;
	std::priority_queue<NodeRecordAs, std::vector<NodeRecordAs>, NodeRecordAsCompare>* open;

	//std::map<Worker*, Task*> workerTaskMap;
	std::vector<Task*> activeTasks;
	//std::map<Worker*, CommanderGoals::CommanderGoal*> workerGoalMap;
	int dedicatedScouts = 5;
	int scoutsPos;
	std::vector<Worker*> scouts;

	// Plan
	std::vector<GoalStep*> availableSteps;
	std::vector<Goal*> goals;

	float replanTimer = 0;
	const float replanDelay = 1;

	Commander();
	~Commander();

	void DefineAvailableTasks();
	void Update(float dTime);
	void UpdatePlan();
	void DebugDraw();
	void ScoutPos(float posX, float posY);

	Worker* FindFreeWorker(EWorkerRole roleConstrain);
	void AssignTask(Worker* worker, Task* task);
};