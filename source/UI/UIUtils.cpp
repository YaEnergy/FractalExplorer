#include "UI/UIUtils.h"

bool IsCircleHovered(Vector2 position, float radius)
{
	return CheckCollisionPointCircle(GetMousePosition(), position, radius);
}

bool IsCirclePressed(Vector2 position, float radius)
{
	return IsCircleHovered(position, radius) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

bool IsRectangleHovered(Rectangle rectangle)
{
	return CheckCollisionPointRec(GetMousePosition(), rectangle);
}

bool IsRectanglePressed(Rectangle rectangle)
{
	return IsRectangleHovered(rectangle) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}