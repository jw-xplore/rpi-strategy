#include "ScoutManager.h"
#include "SystemsHolder.h"
#include "EntityManager.h"
#include "Worker.h"

ScoutManager::ScoutManager()
{
	scouts.reserve(dedicatedScouts);
	std::vector<Worker>* workers = &SystemsHolder::GetInstance()->entityMananger->workers;
	int population = workers->size();

	scoutsPos = population - dedicatedScouts;

	for (int i = scoutsPos; i < population++; i++)
	{
		scouts.push_back(&workers->at(i));
	}
}

void ScoutManager::Update(float dt)
{
	for (Worker*& scout : scouts)
	{
		if (scout->role != EWorkerRole::Scout)
			continue;


	}
}