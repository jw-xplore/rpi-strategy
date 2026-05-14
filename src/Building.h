#pragma once
#include "raylib.h"
#include "Capital.h"
#include <vector>
#include "types.h"

class Worker;

namespace Capital
{
	struct CapitalAmounts;
}

/*
enum EBuildingType
{
	CoalMile,
	ArsmithsForge,
	Smelter,
	TrainingCamp,
	EBuildingTypeCount
};
*/

enum EBuildingState
{
	Preplaced,
	InProgress,
	Finished
};

/*
struct CapitalReservation
{
	Worker* owner;
	Capital::CapitalAmounts reserved;
};
*/

class Building
{
private:

public:
	EBuildingType type;
	Capital::CapitalAmounts storedCapital;
	Capital::CapitalAmounts reservedCapital = Capital::CapitalAmounts();
	float creationTarget;
	float creationCounter = 0;
	EBuildingState state = EBuildingState::Preplaced;
	//bool standing = false; // Building which is not standing is yet to be build
	//std::vector<CapitalReservation> capitalReservations;

	Vector2 position;
	Color color;

	Building() {}
	Building(EBuildingType type, Vector2 startPos, EBuildingState state = EBuildingState::Preplaced);
	~Building() {}

	void StartBuilding();
	void FinishBuilding();
	void ProgressBuilding(float progress);
	Capital::CapitalAmounts GetAvailableCapital();
};

