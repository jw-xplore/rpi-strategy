#include "Building.h"
#include "Database.h"

Building::Building(EBuildingType type, Vector2 startPos, EBuildingState state)
{
	position = startPos;

	this->type = type;
	color = GREEN;
	creationTarget = GameDB::Database::Instance()->actionCostsBuilding[type].time;

	this->state = state;
	if (state == EBuildingState::Finished)
	{
		creationCounter = creationTarget;
		ProgressBuilding(0);
	}

	//storedCapital.amounts[1] = 10;
}

void Building::StartBuilding()
{
	reservedCapital += GameDB::Database::Instance()->actionCostsBuilding[type].capital;
}

void Building::FinishBuilding()
{
	// Substract resources
	reservedCapital -= GameDB::Database::Instance()->actionCostsBuilding[type].capital;
	storedCapital -= GameDB::Database::Instance()->actionCostsBuilding[type].capital;

	// Change building
	creationCounter = creationTarget;
	ProgressBuilding(0);
}

void Building::ProgressBuilding(float progress)
{
	creationCounter += progress;
	state = EBuildingState::InProgress;
	color = BLACK;

	if (creationCounter < creationTarget)
		return;

	// Create building
	switch (type)
	{
	case EBuildingType::CoalMile: color = PURPLE; break;
	case EBuildingType::ArsmithsForge: color = RED; break;
	case EBuildingType::Smelter: color = BLUE; break;
	case EBuildingType::TrainingCamp: color = ORANGE; break;
	}

	state = EBuildingState::Finished;
}

Capital::CapitalAmounts Building::GetAvailableCapital()
{
	Capital::CapitalAmounts availabe = storedCapital;
	availabe -= reservedCapital;

	return availabe;
}