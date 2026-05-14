#pragma once
#include <vector>

class Worker;

class ScoutManager
{
private:
	int dedicatedScouts = 6;

public:
	int scoutsPos;
	std::vector<Worker*> scouts;

	ScoutManager();
	void Update(float dt);
};

