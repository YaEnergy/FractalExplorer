// Fractal Explorer.cpp : Defines the entry point for the application.
//

#include "Fractal Explorer.h"
#include "Mandelbrot.h"
#include "raylib.h"
#include <vector>

//I prefer to use doubles for position here since it gives more precision

double positionX = 0.0;
double positionY = 0.0;

float zoomDeltaTime = 0.0f;

double zoom = 100.0;
double precision = 2.0;
int maxIterations = 20;

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
	zoomDeltaTime += deltaTime;

	//Very basic camera movement
	float movementSpeed = IsKeyDown(KEY_LEFT_SHIFT) ? 500.0f : 100.0f;

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
	{
		if (IsKeyPressed(KEY_I))
			zoomDeltaTime = 0.05f;

		while (zoomDeltaTime >= 0.05)
		{
			zoom *= 1.1;
			zoomDeltaTime -= 0.05;
		}
	}
	else if (IsKeyDown(KEY_O))
	{
		if (IsKeyPressed(KEY_O))
			zoomDeltaTime = 0.05f;

		while (zoomDeltaTime >= 0.05)
		{
			zoom /= 1.1;
			zoomDeltaTime -= 0.05;
		}
	}

	//Very basic precision
	if (IsKeyDown(KEY_Y))
		precision += 1.0 * deltaTime;
	else if (IsKeyDown(KEY_U))
		precision -= 1.0 * deltaTime;

	//Iteration keys
	if (IsKeyPressed(KEY_KP_ADD))
		maxIterations++;
	else if (IsKeyPressed(KEY_KP_SUBTRACT))
		maxIterations--;

	//Draw
	BeginDrawing();
	ClearBackground(BLACK);

	//generate fractal iteration palette
	std::vector<Color> paletteColors = std::vector<Color>(maxIterations);

	for (int i = 0; i < maxIterations; i++)
	{
		paletteColors[i] = ColorFromHSV((i * 15) % 360, 1.0f, 1.0f);
	}

	//draw fractal
	for (int y = 0; y <= screenHeight; y++)
	{
		for (int x = 0; x <= screenWidth; x++)
		{
			int iterations = GetMandelbrotSetComplexNumberMaxIterations({ ((double)x - (double)screenWidth / 2.0) / zoom + positionX, ((double)y - (double)screenHeight / 2.0) / zoom + positionY }, maxIterations);

			DrawPixel(x, y, paletteColors[iterations - 1]);
		}
	}

	DrawFPS(10, 10);
	DrawText(TextFormat("Position : %f, %f", (float)positionX, (float)positionY), 10, 10 + 24, 24, GREEN);
	DrawText(TextFormat("Zoom: %f", zoom), 10, 10 + 24 * 2, 24, GREEN);
	DrawText(TextFormat("Max iterations: %i", maxIterations), 10, 10 + 24 * 3, 24, GREEN);

	EndDrawing();
}