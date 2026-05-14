#pragma once
#include <raylib.h>

class CustomCamera
{
public:
	Camera2D camera;
	float moveSpeed = 1;
	float zoomSpeed = 0.01f;

	CustomCamera();
	void Update(float dt);
};