#pragma once
#include <vector>
#include <map>
#include <tuple>
#include <functional>
#include "Capital.h"
#include <string>

class Worker;
class Building;
enum EWorkerRole;
class EntityManager;

// Main classses
class Subtask;
class Task;
class GoalStep;
class Goal;

enum ESubtaskState
{
	Running,
	Finnished,
	Skipped,
	Canceled,
	ESubtaskStateCount
};

using SubTaskFn = std::function<ESubtaskState(Worker&, float dTime)>;

/// <summary>
/// Single purpose action like arrive, pickup, craft/create a material
/// </summary>
class Subtask
{
public:
	Subtask() {}
	~Subtask() {}

	virtual ESubtaskState Execute(Worker& worker, float dTime) { return ESubtaskState::Finnished; }

};

enum class ETaskAttributeCategory
{
	None = -1,
	Capital,
	Worker,
	Building,
};

struct TaskAttribute
{
	ETaskAttributeCategory category = ETaskAttributeCategory::None;
	int type = -1; // Id of type to given category
	int amount = 0;
	Building* source = nullptr; // Nullptr will result in checking availability in world
	bool blocker = false;

	TaskAttribute() {}

	TaskAttribute(ETaskAttributeCategory category, int type, int amount, Building* source, bool blocker = false): 
		category(category),
		type(type),
		amount(amount),
		source(source),
		blocker(blocker)
	{ }

	static Building* VariableSource(ETaskAttributeCategory category, int type);
};

/// <summary>
/// Holds several subtasks and manage their step by step execution
/// Define required inputs and rewards
/// </summary>
class Task
{
public:
	bool finished = false;
	bool running = false;
	int currentSubTask = 0;
	bool repeat = false;
	std::string name = "";

	Worker* assignee = nullptr;
	std::vector<Subtask*> subtasks;
	GoalStep* parentGoalStep = nullptr;
	std::vector<std::function<void (Task*)>> onFinishedListeners;

	Task() {}
	Task(Worker* worker, std::initializer_list<Subtask*> subtasks);
	Task(const Task& rhs);
	~Task();

	void Update(float dTime);
	void Cancel();
};

/// <summary>
/// Define task type, requirements and output
/// </summary>
class GoalStep
{
public:
	std::string name;

	EWorkerRole roleConstrain;
	std::function<Task* (Worker*)> taskFunc;
	std::vector<TaskAttribute> requirements;
	TaskAttribute output;
	bool variableInOut = false;
	bool isDelivery = false;
	bool doneEvaluateReality = false;

	std::vector<GoalStep*> previousSteps;
	Task* nextStep;
	bool blocker = false; // Blocker won't stop expanding action chain, It is intended to solve blocker input in separate goal chain (e.g. creation of buildings)

	int finishedTasks = 0;
	int totalTasks = 0;
	std::vector<Task*> activeTasks;

	GoalStep() {}
	GoalStep(GoalStep& source);
	GoalStep(std::string name, std::initializer_list<TaskAttribute> requirements, TaskAttribute output, std::function<Task* (Worker*)> taskFunc);

	bool IsAttributeSatisfied(TaskAttribute& attribute, EntityManager* entityMngr);
	bool IsInputSatisfied();
	bool IsActive();
	bool IsDone();
	void AssignTask();
	void OnTaskFinished(Task* task);
};

/// <summary>
/// Holds chain of steps defining completion of complex tasks
/// </summary>
class Goal
{
public:
	unsigned short priority;
	GoalStep* finalStep; // Linked list to previous tasks, finishing this task will complete the goal 

	Goal();
	Goal(GoalStep& finalStep, std::vector<GoalStep*>& availableSteps);

	void DefineTaskChain(GoalStep& currentStep, std::vector<GoalStep*>& availableSteps);
	GoalStep* NextAvailableStep();
	GoalStep* NextAvailableStep(GoalStep& currentStep);
	bool IsCompleted();

	void DebugDraw(GoalStep& step, int posX, int posY);
};

