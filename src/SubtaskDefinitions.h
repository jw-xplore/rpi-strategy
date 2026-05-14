#pragma once
#include <raylib.h>
#include "GoalSystem.h"

enum ESubtaskState;
enum EBuildingType;

class Worker;
enum EWorkerRole;
struct TreesTile;
class Building;
class Pickup;
class World;
class PathFinding;

namespace Capital
{
	enum ECapitalType;
}

namespace SubtaskDefinitions
{
	/*
	// Role task
	ESubtaskState MakeCoal(Worker& worker, float dTime); // Coal miner
	ESubtaskState MakeIronBar(Worker& worker, float dTime); // Smelter operator
	ESubtaskState MakeSword(Worker& worker, float dTime); // Arm smith
	ESubtaskState Build(Worker& worker, float dTime, EBuildingType type); // Builder
	*/

	/// <summary>
	/// Move to specific position. Execute will automaticaly search and start moving along path upon finding.
	/// </summary>
	class MoveToSubtask : public Subtask
	{
	public:
		Vector2 position;
		bool pathAssigned = false;

		MoveToSubtask(Vector2 pos) : position(pos) {}
		ESubtaskState Execute(Worker& worker, float dTime) override;
	};

	/// <summary>
	/// Turn tree tile into felled tree pickup after certain time delay
	/// </summary>
	class FellTreeSubtask : public Subtask
	{
	public:
		TreesTile* treesTile;
		float timer = 0;
		float delay;
		
		int ogX = 0;
		int ogY = 0;

		FellTreeSubtask(TreesTile* tile, float t);
		ESubtaskState Execute(Worker& worker, float dTime) override;
	};

	/// <summary>
	/// Turns pickup into worker carried material.
	/// </summary>
	class PickupSubtask : public Subtask
	{
	public:
		Pickup* pickup;

		PickupSubtask(Pickup* pickup) : pickup(pickup) {}
		ESubtaskState Execute(Worker& worker, float dTime) override;
	};

	class PickupFromBuildingSubtask : public Subtask
	{
	public:
		Building* building;
		Capital::ECapitalType type;

		PickupFromBuildingSubtask(Building* building, Capital::ECapitalType type) : building(building), type(type) {}
		ESubtaskState Execute(Worker& worker, float dTime) override;
	};

	/// <summary>
	/// Turns workers carried material into pickup
	/// </summary>
	class DropItemSubtask : public Subtask
	{
	public:
		Building* targetBuilding;

		DropItemSubtask(Building* target) : targetBuilding(target) {}
		ESubtaskState Execute(Worker& worker, float dTime) override;
	};

	/// <summary>
	/// Changes regular worker role after certain delay
	/// </summary>
	class TrainWorker : public Subtask
	{
	public:
		EWorkerRole role;
		float timer = 0;
		float delay;

		TrainWorker(EWorkerRole role, float t) : role(role), timer(0), delay(t) {}
		ESubtaskState Execute(Worker& worker, float dTime) override;
	};

	/// <summary>
	/// Finish pre-place building after certain delay
	/// </summary>
	class CreateBuilding : public Subtask
	{
	public:
		Building* building;
		bool started = false;
		float timer = 0;
		float delay;

		CreateBuilding(Building* building, float t) : building(building), delay(t) {}
		ESubtaskState Execute(Worker& worker, float dTime) override;
	};

	/// <summary>
	/// Exchange capital into specific item over the time
	/// </summary>
	class CreateItem : public Subtask
	{
	public:
		Building* building;
		bool started = false;
		float timer = 0;
		float delay;
		Capital::CapitalAmounts cost;
		Capital::ECapitalType gainItem;

		CreateItem(Building* building, float t, Capital::CapitalAmounts cost, Capital::ECapitalType gainItem) : building(building), delay(t), cost(cost), gainItem(gainItem) {}
		ESubtaskState Execute(Worker& worker, float dTime) override;
	};

	/// <summary>
	/// Go to closest undiscovered connection
	/// </summary>
	class ScoutSubtask : public Subtask
	{
	public:
		int x, y;
		bool foundPath = false;
		Vector2 target;

		ScoutSubtask(int x, int y): x(x), y(y) {}
		ESubtaskState Execute(Worker& worker, float dTime) override;
		bool FindUndiscoveredTile(int x, int y, PathFinding& pf, World& world);
	};
};

