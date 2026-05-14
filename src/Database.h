#pragma once
#include "Json.hpp"
#include "Capital.h"

namespace GameDB
{
	//--------------------------------------------------
	// Key enums
	//--------------------------------------------------

	enum EActionResource
	{
		FellTree,
		MakeCoal,
		MakeIronBar,
		MakeSword,
		EActionResourceCount
	};

	enum EActionTraining
	{
		TrainScout,
		TrainBuilder,
		TrainSoldier,
		TrainCraftsman,
		EActionTrainingCount
	};

	/*
	enum EActionBuilding
	{
		BuildCoalMile,
		BuildForge,
		BuildSmelter,
		BuildTrainingCamp,
		EActionBuildingCount
	};
	*/

	//--------------------------------------------------
	// Database structures
	//--------------------------------------------------

	struct TerrainData
	{
		//ETerrainType type;
		//const char* name;
		char charIdentifier;
		float cost;
	};

	//--------------------------------------------------
	// Main storage definition
	//--------------------------------------------------
	class Database
	{
	private:
		static Database* instance;

	public:
		int startingPopulation;
		int ironOreAmount;

		TerrainData* terrains;
		Capital::ActionCost* actionCostsResources;
		Capital::ActionCost* actionCostsTraining;
		Capital::ActionCost* actionCostsBuilding;

		Database();
		~Database();

		static Database* Instance()
		{
			if (!instance)
				instance = new Database();

			return instance;
		}

		Capital::ActionCost* DefineActionCosts(nlohmann::ordered_json& j, const char* section, int size);
	};
}

