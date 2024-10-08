#pragma once

#include "raylib.h"

namespace Explorer
{
	float GetScreenScale(float designWidth, float designHeight);

	bool IsCircleHovered(Vector2 position, float radius);

	bool IsCircleDown(Vector2 position, float radius);

	bool IsCirclePressed(Vector2 position, float radius);

	bool IsRectangleHovered(Rectangle rectangle);

	bool IsRectangleDown(Rectangle rectangle);

	bool IsRectanglePressed(Rectangle rectangle);

	//Draws a texture button
	void DrawTextureButton(Texture texture, Rectangle dest, Color idleColor, Color hoverColor, Color pressColor);

	float GetFontSizeForWidth(Font font, const char* text, float width, float spacingMultiplier = 0.1f);
}
