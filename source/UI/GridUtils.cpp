#include "UI/GridUtils.h"

#include <cmath>

#include "raylib.h"

float GetClosestMultipleOf(float val, float factor)
{
	return fmod(val, factor) >= factor / 2.0f ? val - fmod(val, factor) + factor : val - fmod(val, factor);
}

float GetClosestLargerMultipleOf(float val, float factor)
{
	return val - fmod(val, factor) + factor;
}

float GetClosestSmallerMultipleOf(float val, float factor)
{
	return val - fmod(val, factor);
}

Vector2 SnapTo2DGrid(Vector2 position, Vector2 increment)
{
	float snapX = GetClosestMultipleOf(position.x, increment.x);
	float snapY = GetClosestMultipleOf(position.y, increment.y);

	return Vector2{ snapX, snapY };
}

Vector2 SnapTo2DGrid(Vector2 position, float incrementXY)
{
	float snapX = GetClosestMultipleOf(position.x, incrementXY);
	float snapY = GetClosestMultipleOf(position.y, incrementXY);

	return Vector2{ snapX, snapY };
}