#include "UI/UIUtils.h"

#include <iostream>

#include "raylib.h"

namespace Explorer
{
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

	void DrawTextureButton(Texture texture, Rectangle dest, Color idleColor, Color hoverColor, Color pressColor)
	{
		Color color = idleColor;

		if (IsRectangleHovered(dest) && IsMouseButtonDown(MOUSE_BUTTON_LEFT))
			color = pressColor;
		else if (IsRectangleHovered(dest))
			color = hoverColor;

		DrawTexturePro(texture, Rectangle{ 0.0f, 0.0f, (float)texture.width, (float)texture.height }, dest, Vector2 {0.0f, 0.0f}, 0.0f, color);
	}

	float GetFontSizeForWidth(Font font, const char* text, float width, float spacingMultiplier)
	{
		const float BASE_FONT_SIZE = 16.0f;
		return BASE_FONT_SIZE / MeasureTextEx(font, text, BASE_FONT_SIZE, BASE_FONT_SIZE * spacingMultiplier).x * width;
	}
}
