// Fractal Explorer.cpp : Defines the entry point for the application.
//

#include "Fractal Explorer.h"
#include "Mandelbrot.h"
#include "raylib.h"

#include <string>
#include <vector>

//I prefer to use doubles for position here since it gives more precision

double positionX = 0.0;
double positionY = 0.0;

double zoom = 100.0;
float zoomDeltaTime = 0.0f;

double precision = 2.0;
int maxIterations = 20;

RenderTexture fractalRenderTexture;
int qualityDivision = 1;
bool useAntiAliasing = false;

bool showDebugInfo = false;

void InitQualityDivision(int startValue);

void SetQualityDivision(int value);

void DrawMandelbrotFractal(int width, int height, double positionX, double positionY, double zoom, int maxIterations);

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

	InitQualityDivision(1);

	SetWindowState(FLAG_WINDOW_RESIZABLE);

	//TODO: Emscripten modifications
	SetTargetFPS(120);

	while (!WindowShouldClose())
	{
		UpdateDrawFrame();
	}

	//Deinit
	UnloadRenderTexture(fractalRenderTexture);

	CloseWindow();
	CloseAudioDevice();

	return 0;
}

void InitQualityDivision(int startValue)
{
	qualityDivision = startValue;

	fractalRenderTexture = LoadRenderTexture(GetScreenWidth() / startValue, GetScreenHeight() / startValue);

	if (useAntiAliasing)
		SetTextureFilter(fractalRenderTexture.texture, TEXTURE_FILTER_BILINEAR);
	else
		SetTextureFilter(fractalRenderTexture.texture, TEXTURE_FILTER_POINT);
}

void SetQualityDivision(int value)
{
	qualityDivision = value;

	UnloadRenderTexture(fractalRenderTexture);
	fractalRenderTexture = LoadRenderTexture(GetScreenWidth() / value, GetScreenHeight() / value);

	if (useAntiAliasing)
		SetTextureFilter(fractalRenderTexture.texture, TEXTURE_FILTER_BILINEAR);
	else
		SetTextureFilter(fractalRenderTexture.texture, TEXTURE_FILTER_POINT);
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

	//Quality division keys
	if (IsKeyPressed(KEY_W))
		SetQualityDivision(qualityDivision + 1);
	else if (IsKeyPressed(KEY_S) && qualityDivision > 1)
		SetQualityDivision(qualityDivision - 1);

	if (IsKeyPressed(KEY_E))
	{
		useAntiAliasing = !useAntiAliasing;

		if (useAntiAliasing)
			SetTextureFilter(fractalRenderTexture.texture, TEXTURE_FILTER_BILINEAR);
		else
			SetTextureFilter(fractalRenderTexture.texture, TEXTURE_FILTER_POINT);
	}

	if (IsKeyPressed(KEY_SPACE))
		showDebugInfo = !showDebugInfo;

	//Draw
	BeginDrawing();
	ClearBackground(BLACK);

	BeginTextureMode(fractalRenderTexture);
	DrawMandelbrotFractal(screenWidth / qualityDivision, screenHeight / qualityDivision, positionX, positionY, zoom, maxIterations);
	EndTextureMode();

	//render texture is flipped because of OpenGL
	Rectangle fractalSource = { 0.0f, 0.0f, (float)fractalRenderTexture.texture.width, -(float)fractalRenderTexture.texture.height };
	Rectangle fractalDestination = { 0.0f, 0.0f, (float)screenWidth, (float)screenHeight };

	DrawTexturePro(fractalRenderTexture.texture, fractalSource, fractalDestination, { 0.0f, 0.0f }, 0.0f, WHITE);

	if (showDebugInfo)
	{
		DrawFPS(10, 10);
		DrawText(TextFormat("Position : %f, %f", (float)positionX, (float)positionY), 10, 10 + 24, 24, GREEN);
		DrawText(TextFormat("Zoom: %f", zoom), 10, 10 + 24 * 2, 24, GREEN);
		DrawText(TextFormat("Max iterations: %i", maxIterations), 10, 10 + 24 * 3, 24, GREEN);
		DrawText(TextFormat("Quality division: %i", qualityDivision), 10, 10 + 24 * 4, 24, GREEN);
		DrawText(TextFormat("Screen size: %ix%i", screenWidth, screenHeight), 10, 10 + 24 * 5, 24, GREEN);
		DrawText(TextFormat("Fractal size: %ix%i", screenWidth / qualityDivision, screenHeight / qualityDivision), 10, 10 + 24 * 6, 24, GREEN);
		DrawText(TextFormat("Anti-Aliasing?: %s", std::to_string(useAntiAliasing).c_str()), 10, 10 + 24 * 7, 24, GREEN);
	}

	EndDrawing();
}