#pragma once

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

enum EBuildingType
{
	CoalMile,
	ArsmithsForge,
	Smelter,
	TrainingCamp,
	EBuildingTypeCount
};

namespace Capital
{
	enum ECapitalType
	{
		None = -1,
		Tree,
		Coal,
		IronOre,
		IronBar,
		Sword,
		ECapitalTypeCount
	};
}