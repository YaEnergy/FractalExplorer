// Fractal Explorer.cpp : Defines the entry point for the application.
//

#include "Fractal Explorer.h"
#include "Mandelbrot.h"
#include "raylib.h"

Vector2 position = { 0.0f, 0.0f };
double zoom = 1.0;
double precision = 1000.0;
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
	const float CAMERA_SPEED = 200.0f;

	//Update

	//Very basic camera movement
	if (IsKeyDown(KEY_LEFT))
		position = { position.x - CAMERA_SPEED * deltaTime, position.y };
	else if (IsKeyDown(KEY_RIGHT))
		position = { position.x + CAMERA_SPEED * deltaTime, position.y };

	if (IsKeyDown(KEY_UP))
		position = { position.x, position.y - CAMERA_SPEED * deltaTime };
	else if (IsKeyDown(KEY_DOWN))
		position = { position.x, position.y + CAMERA_SPEED * deltaTime };

	//Very basic zoom
	if (IsKeyDown(KEY_I))
		zoom += 100.0f * deltaTime;
	else if (IsKeyDown(KEY_O))
		zoom -= 100.0f * deltaTime;

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
			ComplexNumber complex = GetFractalComplexNumber({ (double)(x + position.x - screenWidth / 2.0f) / zoom, (double)(y + position.y - screenHeight / 2.0f) / zoom }, iterations);

			double distance = GetComplexNumberDistance(complex);
			if (distance <= 2.0)
				DrawRectangleRec({ (float)x, (float)y, 1.0f, 1.0f }, ColorFromHSV(distance / 2.0f * 360.0f, 1.0f, 1.0f));
		}
	}

	DrawFPS(10, 10);
	DrawText(TextFormat("Position : %f, %f", position.x, position.y), 10, 10 + 24, 24, GREEN);
	DrawText(TextFormat("Zoom: %f", zoom), 10, 10 + 24 * 2, 24, GREEN);
	DrawText(TextFormat("Iterations: %i", iterations), 10, 10 + 24 * 3, 24, GREEN);

	EndDrawing();
}