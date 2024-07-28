#include "UI/UIUtils.h"

bool IsCircleHovered(Vector2 position, float radius)
{
	return CheckCollisionPointCircle(GetMousePosition(), position, radius);
}

bool IsCirclePressed(Vector2 position, float radius)
{
	return IsCircleHovered(position, radius) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}