﻿// Fractal Explorer.cpp : Defines the entry point for the application.
//

#include "Fractal Explorer.h"

#include <cmath>
#include <string>
#include <vector>

#include "raylib.h"
#include "raymath.h"

#include "Fractals/Mandelbrot.h"
#include "ComplexNumbers/ComplexFloat.h"

struct MandelbrotFractalOptions
{
	Vector2 position;
	float zoom;
	int maxIterations;
};

struct JuliaFractalOptions
{
	Vector2 position;
	ComplexFloat c;
	float zoom;
	int maxIterations;
};

Vector2 position = Vector2{ 0.0f, 0.0f };

Vector2 juliaC = Vector2{ 0.0f, 0.0f };
float juliaPower = 2.0f;

float zoom = 1.0f;
float zoomDeltaTime = 0.0f;

int maxIterations = 20;

RenderTexture fractalRenderTexture;
bool useAntiAliasing = false;

bool showDebugInfo = false;

const int NUM_FRACTAL_TYPES = 2;

enum FractalType
{
	FRACTAL_MANDELBROT = 0,
	FRACTAL_JULIA = 1
};

const char* fractalNames[NUM_FRACTAL_TYPES] =
{
	"Mandelbrot Set Fractal",
	"Julia Set Fractal"
};

FractalType selectedFractalTypeIndex = FRACTAL_MANDELBROT;
Shader fractalShaders[NUM_FRACTAL_TYPES] = { 0 };

void DrawMandelbrotFractal(int width, int height, double positionX, double positionY, double zoom, int maxIterations);

void UpdateDrawFrame();

void SetFractalType(FractalType fractalType);

#ifdef WIN32RELEASE
int WinMain()
{
	return main();
}
#endif

int main()
{
	const int START_WINDOW_WIDTH = 800;
	const int START_WINDOW_HEIGHT = 480;

	//Init
	InitWindow(START_WINDOW_WIDTH, START_WINDOW_HEIGHT, "Fractal Explorer");
	InitAudioDevice();

	fractalRenderTexture = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());

	if (useAntiAliasing)
		SetTextureFilter(fractalRenderTexture.texture, TEXTURE_FILTER_BILINEAR);
	else
		SetTextureFilter(fractalRenderTexture.texture, TEXTURE_FILTER_POINT);

	
	SetWindowState(FLAG_WINDOW_RESIZABLE);
	
	//Load fractal shaders
	fractalShaders[FRACTAL_MANDELBROT] = LoadShader(NULL, "assets/shaders/mandelbrotFractal.frag");
	fractalShaders[FRACTAL_JULIA] = LoadShader(NULL, "assets/shaders/juliaFractal.frag");

	SetFractalType(FRACTAL_MANDELBROT);

	//TODO: Emscripten modifications

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

void SetFractalType(FractalType fractalType)
{
	selectedFractalTypeIndex = fractalType;

	Shader fractalShader = fractalShaders[selectedFractalTypeIndex];
	Vector2 offset = { -0.5f, -0.5f };

	switch (fractalType)
	{
		case FRACTAL_MANDELBROT:
		{
			position = Vector2{ 0.0f, 0.0f };
			maxIterations = 20;
			zoom = 1.0f;

			float widthStretch = 1.0f  / ((float)GetScreenWidth() / (float)GetScreenHeight());

			SetShaderValue(fractalShader, GetShaderLocation(fractalShader, "widthStretch"), &widthStretch, SHADER_UNIFORM_FLOAT);
			SetShaderValue(fractalShader, GetShaderLocation(fractalShader, "maxIterations"), &maxIterations, SHADER_UNIFORM_INT);
			SetShaderValue(fractalShader, GetShaderLocation(fractalShader, "offset"), &offset, SHADER_UNIFORM_VEC2);
			SetShaderValue(fractalShader, GetShaderLocation(fractalShader, "position"), &position, SHADER_UNIFORM_VEC2);
			SetShaderValue(fractalShader, GetShaderLocation(fractalShader, "zoom"), &zoom, SHADER_UNIFORM_FLOAT);
			SetShaderValue(fractalShader, GetShaderLocation(fractalShader, "maxIterations"), &maxIterations, SHADER_UNIFORM_INT);

			break;
		}
		case FRACTAL_JULIA:
			juliaC = Vector2{ 0.0f, 0.0f };
			juliaPower = 2.0f;
			position = Vector2{ 0.0f, 0.0f };
			maxIterations = 20;
			zoom = 1.0f;

			float widthStretch = 1.0f / ((float)GetScreenWidth() / (float)GetScreenHeight());

			SetShaderValue(fractalShader, GetShaderLocation(fractalShader, "c"), &juliaC, SHADER_UNIFORM_VEC2);
			SetShaderValue(fractalShader, GetShaderLocation(fractalShader, "widthStretch"), &widthStretch, SHADER_UNIFORM_FLOAT);
			SetShaderValue(fractalShader, GetShaderLocation(fractalShader, "maxIterations"), &maxIterations, SHADER_UNIFORM_INT);
			SetShaderValue(fractalShader, GetShaderLocation(fractalShader, "offset"), &offset, SHADER_UNIFORM_VEC2);
			SetShaderValue(fractalShader, GetShaderLocation(fractalShader, "position"), &position, SHADER_UNIFORM_VEC2);
			SetShaderValue(fractalShader, GetShaderLocation(fractalShader, "zoom"), &zoom, SHADER_UNIFORM_FLOAT);
			SetShaderValue(fractalShader, GetShaderLocation(fractalShader, "maxIterations"), &maxIterations, SHADER_UNIFORM_INT);
			SetShaderValue(fractalShader, GetShaderLocation(fractalShader, "power"), &juliaPower, SHADER_UNIFORM_FLOAT);

			break;
	}
}

void UpdateDrawFrame()
{
	int screenWidth = GetScreenWidth();
	int screenHeight = GetScreenHeight();

	float deltaTime = GetFrameTime();

	//Fractal type keys
	if (IsKeyPressed(KEY_ONE))
		SetFractalType(FRACTAL_MANDELBROT);
	else if (IsKeyPressed(KEY_TWO))
		SetFractalType(FRACTAL_JULIA);

	Shader fractalShader = fractalShaders[selectedFractalTypeIndex];

	//Camera panning using keys
	float movementSpeed = IsKeyDown(KEY_LEFT_SHIFT) ? 0.05f : 0.01f;

	if (IsKeyDown(KEY_LEFT))
		position.x -= (double)(movementSpeed * deltaTime) / zoom;
	else if (IsKeyDown(KEY_RIGHT))
		position.x += (double)(movementSpeed * deltaTime) / zoom;

	if (IsKeyDown(KEY_UP))
		position.y += (double)(movementSpeed * deltaTime) / zoom;
	else if (IsKeyDown(KEY_DOWN))
		position.y -= (double)(movementSpeed * deltaTime) / zoom;

	//Camera panning using mouse
	if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
	{
		Vector2 mouseDelta = GetMouseDelta();

		position.x -= mouseDelta.x / (float)fractalRenderTexture.texture.width / zoom; /// qualityDivision / zoom;
		position.y += mouseDelta.y / (float)fractalRenderTexture.texture.height / zoom; /// qualityDivision / zoom;
	}

	SetShaderValue(fractalShader, GetShaderLocation(fractalShader, "position"), &position, SHADER_UNIFORM_VEC2);

	//Camera zooming using keys
	zoomDeltaTime += deltaTime;

	if (IsKeyDown(KEY_I))
	{
		if (IsKeyPressed(KEY_I))
			zoomDeltaTime = 0.01f;

		while (zoomDeltaTime >= 0.01f)
		{
			zoom *= 1.02f;
			zoomDeltaTime -= 0.01f;
		}
	}
	else if (IsKeyDown(KEY_O))
	{
		if (IsKeyPressed(KEY_O))
			zoomDeltaTime = 0.01f;

		while (zoomDeltaTime >= 0.01f)
		{
			zoom /= 1.02f;
			zoomDeltaTime -= 0.01f;
		}
	}

	if (selectedFractalTypeIndex == FRACTAL_JULIA)
	{
		//Power changing using keys
		if (IsKeyDown(KEY_F))
			juliaPower -= 1.0f * deltaTime;
		else if (IsKeyDown(KEY_G))
			juliaPower += 1.0f * deltaTime;

		//Round power down
		if (IsKeyPressed(KEY_H))
			juliaPower = floor(juliaPower);

		SetShaderValue(fractalShader, GetShaderLocation(fractalShader, "power"), &juliaPower, SHADER_UNIFORM_FLOAT);

		//c panning using keys
		float movementSpeed = IsKeyDown(KEY_LEFT_SHIFT) ? 0.05f : 0.01f;

		if (IsKeyDown(KEY_A))
			juliaC.x -= (double)(movementSpeed * deltaTime) / zoom;
		else if (IsKeyDown(KEY_D))
			juliaC.x += (double)(movementSpeed * deltaTime) / zoom;

		if (IsKeyDown(KEY_W))
			juliaC.y += (double)(movementSpeed * deltaTime) / zoom;
		else if (IsKeyDown(KEY_S))
			juliaC.y -= (double)(movementSpeed * deltaTime) / zoom;

		//c panning using mouse right click
		if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
		{
			Vector2 mouseDelta = GetMouseDelta();

			juliaC.x -= mouseDelta.x / (float)fractalRenderTexture.texture.width / zoom; /// qualityDivision / zoom;
			juliaC.y += mouseDelta.y / (float)fractalRenderTexture.texture.height / zoom; /// qualityDivision / zoom;
		}

		SetShaderValue(fractalShader, GetShaderLocation(fractalShader, "c"), &juliaC, SHADER_UNIFORM_VEC2);
	}

	//Zooming using mouse wheel
	//up scroll: zoom in
	//down scroll: zoom out
	float mouseWheelMoved = GetMouseWheelMove();
	zoom += zoom * 0.1 * mouseWheelMoved;

	//Zooming using pinching
	Vector2 pinchMovement = GetGesturePinchVector();
	zoom += zoom * 0.5 * (double)Vector2Length(pinchMovement);

	if (zoom <= 0.01f)
		zoom = 0.01f;

	SetShaderValue(fractalShader, GetShaderLocation(fractalShader, "zoom"), &zoom, SHADER_UNIFORM_FLOAT);

	//Iteration keys
	bool iterationsChanged = false;
	if (IsKeyPressed(KEY_KP_ADD))
	{
		maxIterations++;
		iterationsChanged = true;
	}
	else if (IsKeyPressed(KEY_KP_SUBTRACT))
	{
		maxIterations--;
		iterationsChanged = true;
	}

	if (iterationsChanged)
		SetShaderValue(fractalShader, GetShaderLocation(fractalShader, "maxIterations"), &maxIterations, SHADER_UNIFORM_INT);

	//Update fractal render texture if screen size has changed
	if (screenWidth != fractalRenderTexture.texture.width || screenHeight != fractalRenderTexture.texture.height)
	{
		UnloadRenderTexture(fractalRenderTexture);
		fractalRenderTexture = LoadRenderTexture(screenWidth, screenHeight);

		float widthStretch = 1.0f / ((float)GetScreenWidth() / (float)GetScreenHeight());

		SetShaderValue(fractalShader, GetShaderLocation(fractalShader, "widthStretch"), &widthStretch, SHADER_UNIFORM_FLOAT);
	}

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
	{
		ClearBackground(BLACK);

		BeginTextureMode(fractalRenderTexture);
		{
			ClearBackground(BLACK);
			DrawRectangle(0, 0, fractalRenderTexture.texture.width, fractalRenderTexture.texture.height, BLACK);
		}
		EndTextureMode();


		BeginShaderMode(fractalShader);
		{
			//render texture is flipped because of OpenGL
			Rectangle fractalSource = { 0.0f, 0.0f, (float)fractalRenderTexture.texture.width, -(float)fractalRenderTexture.texture.height };
			Rectangle fractalDestination = { 0.0f, 0.0f, (float)screenWidth, (float)screenHeight };

			DrawTexturePro(fractalRenderTexture.texture, fractalSource, fractalDestination, { 0.0f, 0.0f }, 0.0f, WHITE);
		}
		EndShaderMode();

		if (showDebugInfo)
		{
			DrawFPS(10, 10);
			DrawText(TextFormat("Position : %f, %f", position.x, position.y), 10, 10 + 24, 24, GREEN);
			DrawText(TextFormat("Zoom: %f", zoom), 10, 10 + 24 * 2, 24, GREEN);
			DrawText(TextFormat("Max iterations: %i", maxIterations), 10, 10 + 24 * 3, 24, GREEN);
			DrawText(TextFormat("Screen size: %ix%i", screenWidth, screenHeight), 10, 10 + 24 * 4, 24, GREEN);
			DrawText(TextFormat("Anti-Aliasing?: %s", std::to_string(useAntiAliasing).c_str()), 10, 10 + 24 * 5, 24, GREEN);
			DrawText(TextFormat("Pixels: %i", screenWidth * screenHeight), 10, 10 + 24 * 6, 24, GREEN);

			DrawText(fractalNames[selectedFractalTypeIndex], 10, 10 + 24 * 7, 24, WHITE);

			if (selectedFractalTypeIndex == FRACTAL_JULIA)
			{
				DrawText(TextFormat("Julia C : %f, %f", juliaC.x, juliaC.y), 10, 10 + 24 * 8, 24, WHITE);
				DrawText(TextFormat("Julia Pow : %f", juliaPower), 10, 10 + 24 * 9, 24, WHITE);
			}
		}

	}
	EndDrawing();
}