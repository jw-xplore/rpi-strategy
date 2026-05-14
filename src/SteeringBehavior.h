#pragma once
#include "raylib.h"
#include "raymath.h"

namespace SteeringBehavior
{
	inline Vector2 Seek(Vector2 pos, Vector2 targetPos, float speed)
	{
		Vector2 dir = Vector2Subtract(targetPos, pos);
		dir = Vector2Normalize(dir);

		return Vector2Multiply(dir, {speed, speed});
	}
}

