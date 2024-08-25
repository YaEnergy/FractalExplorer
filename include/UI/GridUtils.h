#pragma once

#include "raylib.h"

namespace Explorer
{
	//returns a multiple of factor closest to val
	float GetClosestMultipleOf(float val, float factor);

	//returns a multiple of factor closest to val and larger than val
	float GetClosestLargerMultipleOf(float val, float factor);

	//returns a multiple of factor closest to val and smaller than val
	float GetClosestSmallerMultipleOf(float val, float factor);

	Vector2 SnapTo2DGrid(Vector2 vector, Vector2 increment);
	Vector2 SnapTo2DGrid(Vector2 vector, float incrementXY);
}