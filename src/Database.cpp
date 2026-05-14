#include "Database.h"
#include <fstream>
#include <iostream>
#include <string>

#include "World.h"
#include "Building.h"

using namespace GameDB;

Database* Database::instance;

Database::Database()
{
	// Read json
	std::ifstream file("resources/db.json");
	nlohmann::ordered_json jsonRes = nlohmann::ordered_json::parse(file);
	file.close();

	// Store data--------------------------------

	// Basic data
	startingPopulation = jsonRes["worldData"]["population"];
	ironOreAmount = jsonRes["worldData"]["ironOreAmount"];

	// Terrains
	terrains = new TerrainData[ETerrainType::ETerrainTypeCount];
	for (size_t i = 0; i < ETerrainType::ETerrainTypeCount; i++)
	{
		terrains[i].charIdentifier = jsonRes["terrain"][i]["char"].get<std::string>()[0];
		float travelSpeed = jsonRes["terrain"][i]["travelSpeed"];

		if (travelSpeed > 0)
			terrains[i].cost = 1 / travelSpeed;
		else
			terrains[i].cost = travelSpeed;
	}

	// Costs
	actionCostsResources = DefineActionCosts(jsonRes, "resourceActionCost", EActionResource::EActionResourceCount);
	actionCostsTraining = DefineActionCosts(jsonRes, "trainingActionCost", EActionTraining::EActionTrainingCount);
	actionCostsBuilding = DefineActionCosts(jsonRes, "buildingActionCost", EBuildingType::EBuildingTypeCount);
}

Database::~Database()
{
	delete[] terrains;
	delete[] actionCostsResources;
	delete[] actionCostsTraining;
	delete[] actionCostsBuilding;
}

Capital::ActionCost* Database::DefineActionCosts(nlohmann::ordered_json& j, const char* section, int size)
{
	using namespace Capital;

	ActionCost* costs = new ActionCost[size];
	
	for (size_t i = 0; i < size; i++)
	{
		costs[i].time = j[section][i].at("time");
		costs[i].capital.amounts[ECapitalType::Tree] = j[section][i].at("trees");
		costs[i].capital.amounts[ECapitalType::Coal] = j[section][i].at("coal");
		costs[i].capital.amounts[ECapitalType::IronOre] = j[section][i].at("ironOre");
		costs[i].capital.amounts[ECapitalType::IronBar] = j[section][i].at("ironBar");
		costs[i].capital.amounts[ECapitalType::Sword] = j[section][i].at("swords");
	}

	return costs;
}