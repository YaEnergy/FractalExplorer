﻿// Fractal Explorer.cpp : Defines the entry point for the application.
//

#include "Fractal Explorer.h"
#include "Mandelbrot.h"
#include "raylib.h"

//I prefer to use doubles for position here since it gives more precision

double positionX = 0.0;
double positionY = 0.0;

double zoom = 100.0;
double precision = 2.0;
int iterations = 20;

void UpdateDrawFrame();

#ifdef WIN32RELEASE
int WinMain()
{
	return main();
}
#endif

int main()
{
	std::cout << "Hello CMake, and raylib!" << std::endl;

	//Init
	InitWindow(800, 480, "Fractal Explorer");
	InitAudioDevice();

	SetWindowState(FLAG_WINDOW_RESIZABLE);

	//TODO: Emscripten modifications
	SetTargetFPS(120);

	while (!WindowShouldClose())
	{
		UpdateDrawFrame();
	}

	//Deinit
	CloseWindow();
	CloseAudioDevice();

	return 0;
}

void UpdateDrawFrame()
{
	int screenWidth = GetScreenWidth();
	int screenHeight = GetScreenHeight();

	float deltaTime = GetFrameTime();

	//Update

	//Very basic camera movement
	float movementSpeed = IsKeyDown(KEY_LEFT_SHIFT) ? 100.0f : 10.0f;

	if (IsKeyDown(KEY_LEFT))
		positionX -= (double)(movementSpeed * deltaTime) / zoom;
	else if (IsKeyDown(KEY_RIGHT))
		positionX += (double)(movementSpeed * deltaTime) / zoom;

	if (IsKeyDown(KEY_UP))
		positionY -= (double)(movementSpeed * deltaTime) / zoom;
	else if (IsKeyDown(KEY_DOWN))
		positionY += (double)(movementSpeed * deltaTime) / zoom;

	//Very basic zoom
	if (IsKeyDown(KEY_I))
		zoom *= 1.1;
	else if (IsKeyDown(KEY_O))
		zoom /= 1.1;

	//Very basic precision
	if (IsKeyDown(KEY_Y))
		precision += 1.0 * deltaTime;
	else if (IsKeyDown(KEY_U))
		precision -= 1.0 * deltaTime;

	//Iteration keys
	if (IsKeyPressed(KEY_KP_ADD))
		iterations++;
	else if (IsKeyPressed(KEY_KP_SUBTRACT))
		iterations--;

	//Draw
	BeginDrawing();
	ClearBackground(BLACK);

	for (int y = 0; y <= screenHeight; y++)
	{
		for (int x = 0; x <= screenWidth; x++)
		{
			ComplexNumber complex = GetMandelbrotSetComplexNumber({ ((double)x - (double)screenWidth / 2.0) / zoom + positionX, ((double)y - (double)screenHeight / 2.0) / zoom + positionY }, iterations);

			double distance = complex.GetDistanceFromOrigin();
			if (distance <= 2.0)
				DrawPixel(x, y, ColorFromHSV(distance / 2.0 * 360.0, 1.0f, 1.0f));
		}
	}

	DrawFPS(10, 10);
	DrawText(TextFormat("Position : %f, %f", (float)positionX, (float)positionY), 10, 10 + 24, 24, GREEN);
	DrawText(TextFormat("Zoom: %f", zoom), 10, 10 + 24 * 2, 24, GREEN);
	DrawText(TextFormat("Iterations: %i", iterations), 10, 10 + 24 * 3, 24, GREEN);

	EndDrawing();
}