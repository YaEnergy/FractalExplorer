#pragma once

#include "raylib.h"

bool IsCircleHovered(Vector2 position, float radius);

bool IsCirclePressed(Vector2 position, float radius);

bool IsRectangleHovered(Rectangle rectangle);

bool IsRectanglePressed(Rectangle rectangle);

//Draws a texture button
void DrawTextureButton(Texture texture, Rectangle dest, Color idleColor, Color hoverColor, Color pressColor);

float GetFontSizeForWidth(Font font, const char* text, float width, float spacingMultiplier = 0.1f);
