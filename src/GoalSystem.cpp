#include "GoalSystem.h"
#include "SystemsHolder.h"
#include "World.h";
#include "EntityManager.h"
#include "Building.h"
#include <stdexcept>
#include "Commander.h"
#include "Worker.h"
#include <iostream>
#include "Capital.h"

Building* TaskAttribute::VariableSource(ETaskAttributeCategory category, int type)
{
	// Find correct building for a resource type
	if (category == ETaskAttributeCategory::Capital)
	{
		EntityManager* entityManager = SystemsHolder::GetInstance()->entityMananger;
		entityManager->FindBuildingOfType(EBuildingType::CoalMile);

		switch (type)
		{
		case Capital::ECapitalType::Tree: return nullptr;
		case Capital::ECapitalType::Coal: return entityManager->FindBuildingOfType(EBuildingType::CoalMile);
		case Capital::ECapitalType::IronOre: return nullptr;
		case Capital::ECapitalType::IronBar: return entityManager->FindBuildingOfType(EBuildingType::Smelter);
		case Capital::ECapitalType::Sword: return entityManager->FindBuildingOfType(EBuildingType::ArsmithsForge);
		}
	}

	return nullptr;
}

//--------------------------------------------------------------
// Task
//--------------------------------------------------------------

Task::Task(Worker* worker, std::initializer_list<Subtask*> subtasks)
{
	finished = false;
	running = true;
	currentSubTask = 0;
	repeat = false;
	assignee = worker;

	this->subtasks = subtasks;
}

Task::Task(const Task& rhs)
{

}

Task::~Task()
{
	subtasks.clear();
}

void Task::Update(float dTime)
{
	if (!running)
		return;

	if (currentSubTask >= subtasks.size())
	{
		if (repeat)
		{
			// Repeat
			currentSubTask = 0;
		}
		else
		{
			// Stop execution - All subtasks done
			running = false;
			finished = true;

			for (auto& listener : onFinishedListeners)
			{
				listener(this);
			}
		}

		return;
	}

	// Run and progress subtask
	//ESubtaskState state = subtasks[currentSubTask](*assignee, dTime);
	ESubtaskState state = subtasks[currentSubTask]->Execute(*assignee, dTime);

	if (state == ESubtaskState::Canceled)
	{
		// End task execution
		running = false;
		return;
	}

	if (state == ESubtaskState::Finnished || state == ESubtaskState::Skipped)
		currentSubTask++;
}

void Task::Cancel()
{
	running = true;
}

//--------------------------------------------------------------
// Goal step
//--------------------------------------------------------------

GoalStep::GoalStep(GoalStep& source)
{
	this->name = source.name;

	this->requirements = source.requirements;
	this->output = source.output;
	this->roleConstrain = source.roleConstrain;

	this->taskFunc = std::function<Task* (Worker*)>(source.taskFunc);
	this->totalTasks = source.totalTasks;

	this->variableInOut = source.variableInOut;
	this->isDelivery = source.isDelivery;
	this->doneEvaluateReality = source.doneEvaluateReality;

	if (this->output.category == ETaskAttributeCategory::Worker && this->output.type == EWorkerRole::Soldier)
		int a = 5;
}

GoalStep::GoalStep(std::string name, std::initializer_list<TaskAttribute> requirements, TaskAttribute output, std::function<Task* (Worker*)> taskFunc)
{
	this->name = name;

	this->requirements = requirements;
	this->output = output;

	this->taskFunc = taskFunc;
}

bool GoalStep::IsAttributeSatisfied(TaskAttribute& attribute, EntityManager* entityMngr)
{
	// Capital
	if (attribute.category == ETaskAttributeCategory::Capital)
	{
		Capital::ECapitalType type = (Capital::ECapitalType)attribute.type;

		// For world
		if (!attribute.source)
		{
			// Find single free pickup - In world we don't care about mutliple items
			return entityMngr->FindFreePickupOfType(type);
		}
		else
		{
			// For building
			return attribute.source->GetAvailableCapital()[type] >= attribute.amount;
		}
	}

	// Worker
	if (attribute.category == ETaskAttributeCategory::Worker)
	{
		return entityMngr->FindWorkerOfRole((EWorkerRole)attribute.type);
	}

	// Building
	if (attribute.category == ETaskAttributeCategory::Building)
	{
		return entityMngr->FindFinishedBuildingOfType((EBuildingType)attribute.type);
	}

	return false;
}

bool GoalStep::IsInputSatisfied()
{
	EntityManager* entityMngr = SystemsHolder::GetInstance()->entityMananger;

	for (TaskAttribute& attribute : requirements)
	{
		if (!IsAttributeSatisfied(attribute, entityMngr))
			return false;
	}

	// All fullfilled
	return true;
}

bool GoalStep::IsActive()
{
	if (totalTasks <= 0)
		return false;

	return activeTasks.size() > 0 && finishedTasks < totalTasks;
}

bool GoalStep::IsDone()
{
	// Evaluate a real state
	if (doneEvaluateReality)
	{
		EntityManager* entityMngr = SystemsHolder::GetInstance()->entityMananger;
		return IsAttributeSatisfied(output, entityMngr);
	}

	// Check tasks fullfilment
	if (totalTasks == 0)
		return false;

	//if (finishedTasks > totalTasks)
		//throw std::runtime_error("Goal step finished tasks overflow!");

	return finishedTasks == totalTasks;
}

void GoalStep::AssignTask()
{
	Commander* commander = SystemsHolder::GetInstance()->commander;
	Worker* worker = commander->FindFreeWorker(roleConstrain);

	if (worker)
	{
		if (isDelivery && output.type == Capital::ECapitalType::Coal)
			int a = 4;

		Task* task = taskFunc(worker);
		if (!task)
			return;

		task->parentGoalStep = this;
		commander->AssignTask(worker, task);

		// Push active task
		activeTasks.push_back(task);
		task->onFinishedListeners.push_back(
			[&](Task* task) { OnTaskFinished(task); }
		);
	}
}

void GoalStep::OnTaskFinished(Task* task)
{
	activeTasks.erase(std::remove(std::begin(activeTasks), std::end(activeTasks), task), std::end(activeTasks));
}

//--------------------------------------------------------------
// Goal
//--------------------------------------------------------------

Goal::Goal()
{

}

Goal::Goal(GoalStep& finalStep, std::vector<GoalStep*>& availableSteps)
{
	this->finalStep = new GoalStep(finalStep);
	if (this->finalStep->totalTasks == 0)
		this->finalStep->totalTasks = 1;

	DefineTaskChain(*this->finalStep, availableSteps);
}

void Goal::DefineTaskChain(GoalStep& currentStep, std::vector<GoalStep*>& availableSteps)
{
	// No input criteria needed = Regular worker can do immediately
	if (currentStep.requirements.empty())
		return;

	// Define previous task for each input to safisfy current criteria
	for (const TaskAttribute& input : currentStep.requirements)
	{
		// HACK: Ingore iron ore as there is no previous step required
		if (input.category == ETaskAttributeCategory::Capital && input.type == Capital::ECapitalType::IronOre && currentStep.isDelivery)
		{
			currentStep.requirements.clear();
			continue;
		}

		for (GoalStep*& step : availableSteps)
		{
			// Compare required source
			bool sourcePass = step->output.source == input.source;
			bool inputSafisfied = step->output.category == input.category && step->output.type == input.type && sourcePass;
			if (step->variableInOut)
				inputSafisfied = step->output.category == input.category;

			// Found task satisfying input criteria
			if (inputSafisfied)
			{
				GoalStep* newStep = new GoalStep(*step);

				if (input.blocker)
					int a = 5;

				newStep->blocker = input.blocker;

				if (newStep->variableInOut)
				{
					// Variable input requirements
					if (newStep->requirements.empty())
					{

						newStep->requirements.push_back(TaskAttribute(
							input.category,
							input.type,
							1,
							TaskAttribute::VariableSource(input.category, input.type)
						));

					}

					// Variable output
					if (newStep->output.source == nullptr)
						newStep->output.source = input.source;

					if (newStep->output.category == ETaskAttributeCategory::None)
						newStep->output.category = input.category;

					if (newStep->output.type == -1)
						newStep->output.type = input.type;

					if (newStep->output.amount == 0)
						newStep->output.amount = input.amount;

					// Dynamic task assign for deliver step
					if (newStep->isDelivery)
					{
						if (input.type == Capital::ECapitalType::Coal)
							int a = 5;

						Building* from = TaskAttribute::VariableSource(input.category, input.type);

						if (from)
						{
							// Getting resources from building
							newStep->taskFunc = [*this, &input, from](Worker* worker) {
								//Building* building = TaskAttribute::VariableSource(newStep->output.category, newStep->output.type);
								Building* building = input.source;
								return WorkerTasks::DeliverFromBuildingTask(worker, static_cast<Capital::ECapitalType>(input.type), from, building);
								};
						}
						else
						{
							// Getting resources from world
							newStep->taskFunc = [*this, &input](Worker* worker) {
								//Building* building = TaskAttribute::VariableSource(newStep->output.category, newStep->output.type);
								Building* building = input.source;
								return WorkerTasks::DeliverItemTask(worker, static_cast<Capital::ECapitalType>(input.type), building);
								};
						}
					}
				}



				newStep->totalTasks = input.amount;
				
				// Mutliply supplies
				if (newStep->output.category == ETaskAttributeCategory::Capital)
					newStep->totalTasks  *= currentStep.totalTasks;

				currentStep.previousSteps.push_back(newStep);

				std::cout << "new step: " << newStep->name << " (from: " << currentStep.name << ")" << "\n";

				if (!newStep->blocker)
					DefineTaskChain(*newStep, availableSteps);
				break;
			}
		}
	}
}

GoalStep* Goal::NextAvailableStep()
{
	return NextAvailableStep(*finalStep);
}

GoalStep* Goal::NextAvailableStep(GoalStep& currentStep)
{
	// This is done - Should work only with first step in goal chain = goal is done
	if (currentStep.IsDone())
		return nullptr;

	// Can perform this step?
	if (currentStep.IsInputSatisfied())
	{
		//if (currentStep.name == "Cr coal")
			//int a = 5;

		// Is there work to be done? 
		int assigned = currentStep.activeTasks.size() + currentStep.finishedTasks;
		if (assigned < currentStep.totalTasks)
			return &currentStep;
	}

	// Check not started previous tasks
	GoalStep* available = nullptr;

	for (GoalStep*& step : currentStep.previousSteps)
	{
		if (!step->IsDone() && !step->blocker)
		{
			available = NextAvailableStep(*step);
			if (available)
				break;
		}
	}

	// Is active and fully occupied - wait as nothing can be done now
	return available;
}

void Goal::DebugDraw(GoalStep& step, int posX, int posY)
{
	// Helper stats
	std::string amount = " (" + std::to_string(step.totalTasks) + ")";

	std::string buildingStr = "Non";
	if (step.output.source)
		buildingStr = std::to_string(step.output.source->type);

	std::string outCTStr = std::to_string((int)step.output.category) + " - " + std::to_string(step.output.type);

	std::string taskStr = " " + std::to_string(step.finishedTasks) + "(" + std::to_string(step.activeTasks.size()) + ")/" + std::to_string(step.totalTasks);

	// Display string
	std::string str = step.name + amount /* + "\n out bu:" + buildingStr */+ "\n out c/t:" + outCTStr  + taskStr;
	char const* cZoom = str.c_str();

	Color coloring = RED;
	if (step.IsActive())
		coloring = YELLOW;
	if (step.IsDone())
		coloring = GREEN;

	DrawText(cZoom, posX * 250, posY * 100, 20, coloring);

	int addY = 0;

	for (GoalStep*& prev : step.previousSteps)
	{
		DebugDraw(*prev, posX + 1, posY + addY);
		addY++;
	}
}