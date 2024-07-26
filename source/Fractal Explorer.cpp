﻿// Fractal Explorer.cpp : Defines the entry point for the application.
//

#include "Fractal Explorer.h"

#include <cmath>
#include <string>
#include <vector>
#include <filesystem>

#include "raylib.h"
#include "raymath.h"

#include "Fractal.h"

const char* fractalEquations[NUM_FRACTAL_TYPES] =
{
	"z ^ 2 + c",
	"(Re(z) - Im(z) i) ^ 2 + c",
	"(|Re(z) + |Im (z)| i) ^ 2 + c",
	"z ^ n + c",
	"z ^ n + c"
};

FractalParameters fractalParameters = FractalParameters();
ShaderFractal shaderFractal;
bool showDebugInfo = false;

//Delta times
float zoomDeltaTime = 0.0f;

void UpdateDrawFrame();

void Update();
void Draw();

#pragma region Fractal functions
Vector2 GetScreenToFractalPosition(Vector2 screenPosition);
Vector2 GetFractalToScreenPosition(Vector2 fractalPosition);

void ChangeFractal(FractalType fractalType);

void ResetFractalParameters();

void UpdateFractal();
void UpdateFractalControls();
void UpdateFractalCamera();
#pragma endregion

#pragma region UI functions
float GetFontSizeForWidth(Font font, const char* text, float width, float spacingMultiplier = 0.1f);

void DrawFractalGridAxises();

void UpdateUI();
void DrawUI();
#pragma endregion


#ifdef WIN32RELEASE
int WinMain()
{
	return main();
}
#endif

int main()
{
	//Init
	InitWindow(DESIGN_WIDTH, DESIGN_HEIGHT, "Complex Fractal Explorer");
	InitAudioDevice();

	SetWindowState(FLAG_WINDOW_RESIZABLE);
	SetExitKey(KEY_NULL);

	//Set up icon
	Image windowIcon = LoadImage("assets/fractalExplorerIcon.png");
	ImageFormat(&windowIcon, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8); //Required for window icon
	SetWindowIcon(windowIcon);
	UnloadImage(windowIcon);
	
	//Fractal set up
	InitFractalRenderTexture(GetScreenWidth(), GetScreenHeight());
	
	fractalParameters.type = FRACTAL_MANDELBROT;
	shaderFractal = LoadShaderFractal(FRACTAL_MANDELBROT);

	shaderFractal.SetNormalizedScreenOffset(Vector2{ -0.5f, -0.5f });
	float widthStretch = 1.0f / ((float)GetScreenWidth() / (float)GetScreenHeight());
	shaderFractal.SetWidthStretch(widthStretch);

	ResetFractalParameters();

	//TODO: Emscripten modifications

	while (!WindowShouldClose())
	{
		UpdateDrawFrame();
	}

	//Deinit
	UnloadFractalRenderTexture();
	shaderFractal.Unload();

	CloseAudioDevice();
	CloseWindow();

	return 0;
}

void UpdateDrawFrame()
{
	Update();

	Draw();
}

void Update()
{
	UpdateFractal();

	if (IsKeyPressed(KEY_SPACE))
		showDebugInfo = !showDebugInfo;

	if (IsKeyPressed(KEY_J))
		SaveShaderFractalToImage(shaderFractal);
}

void Draw()
{
	BeginDrawing();
	{
		ClearBackground(BLACK);

		shaderFractal.Draw(Rectangle{0.0f, 0.0f, (float)GetScreenWidth(), (float)GetScreenHeight()});

		DrawUI();
	}
	EndDrawing();
}

#pragma region Fractal function implementations
Vector2 GetScreenToFractalPosition(Vector2 screenPosition)
{
	int screenWidth = GetScreenWidth();
	int screenHeight = GetScreenHeight();

	Vector2 normalizedScreenOffset = Vector2{ -0.5f, -0.5f };
	Vector2 fragTexCoord = Vector2{ screenPosition.x / (float)screenWidth, screenPosition.y / (float)screenHeight };
	float widthStretch = 1.0f / ((float)GetScreenWidth() / (float)GetScreenHeight());

	Vector2 fractalPosition = Vector2{
		(fragTexCoord.x + normalizedScreenOffset.x) / (widthStretch * fractalParameters.zoom) + fractalParameters.position.x,
		(fragTexCoord.y + normalizedScreenOffset.y) / -fractalParameters.zoom + fractalParameters.position.y
	};

	return Vector2{ fractalPosition.x, fractalPosition.y };
}

Vector2 GetFractalToScreenPosition(Vector2 fractalPosition)
{
	int screenWidth = GetScreenWidth();
	int screenHeight = GetScreenHeight();

	Vector2 normalizedScreenOffset = Vector2{ -0.5f, -0.5f };
	float widthStretch = 1.0f / ((float)GetScreenWidth() / (float)GetScreenHeight());

	Vector2 screenPosition = Vector2{
		((fractalPosition.x - fractalParameters.position.x) * widthStretch * fractalParameters.zoom - normalizedScreenOffset.x) * (float)screenWidth,
		(-(fractalPosition.y - fractalParameters.position.y) * fractalParameters.zoom - normalizedScreenOffset.y) * (float)screenHeight
	};

	return screenPosition;
}

void ChangeFractal(FractalType fractalType)
{
	fractalParameters.type = fractalType;

	shaderFractal.Unload();
	shaderFractal = LoadShaderFractal(fractalType);

	shaderFractal.SetNormalizedScreenOffset(Vector2{ -0.5f, -0.5f });
	float widthStretch = 1.0f / ((float)GetScreenWidth() / (float)GetScreenHeight());
	shaderFractal.SetWidthStretch(widthStretch);

	ResetFractalParameters();
}

void ResetFractalParameters()
{
	fractalParameters.position = Vector2{ 0.0f, 0.0f };
	shaderFractal.SetPosition(fractalParameters.position);

	fractalParameters.zoom = 1.0f;
	shaderFractal.SetZoom(fractalParameters.zoom);

	fractalParameters.maxIterations = 300;
	shaderFractal.SetMaxIterations(fractalParameters.maxIterations);

	fractalParameters.c = Vector2{ 0.0f, 0.0f };

	if (fractalParameters.type == FRACTAL_JULIA)
		shaderFractal.SetC(fractalParameters.c);

	if (fractalParameters.type == FRACTAL_MULTIBROT || fractalParameters.type == FRACTAL_MULTICORN)
		fractalParameters.power = 3.0f;
	else
		fractalParameters.power = 2.0f;

	if (fractalParameters.type == FRACTAL_MULTIBROT || fractalParameters.type == FRACTAL_JULIA || fractalParameters.type == FRACTAL_MULTICORN)
		shaderFractal.SetPower(fractalParameters.power);
}

void UpdateFractal()
{
	int screenWidth = GetScreenWidth();
	int screenHeight = GetScreenHeight();

	if (IsKeyPressed(KEY_ONE))
		ChangeFractal(FRACTAL_MANDELBROT);
	else if (IsKeyPressed(KEY_TWO))
		ChangeFractal(FRACTAL_TRICORN);
	else if (IsKeyPressed(KEY_THREE))
		ChangeFractal(FRACTAL_BURNING_SHIP);
	else if (IsKeyPressed(KEY_FOUR))
		ChangeFractal(FRACTAL_JULIA);
	else if (IsKeyPressed(KEY_FIVE))
		ChangeFractal(FRACTAL_MULTIBROT);
	else if (IsKeyPressed(KEY_SIX))
		ChangeFractal(FRACTAL_MULTICORN);

	if (IsKeyPressed(KEY_T))
		ChangeFractal((FractalType)(((int)fractalParameters.type + 1) % NUM_FRACTAL_TYPES));

	if (IsKeyPressed(KEY_E))
	{
		fractalParameters.colorBanding = !fractalParameters.colorBanding;
		shaderFractal.SetColorBanding(fractalParameters.colorBanding);
	}

	//Update fractal render texture if screen size has changed
	if (screenWidth != GetFractalRenderTextureWidth() || screenHeight != GetFractalRenderTextureHeight())
	{
		SetFractalRenderTextureSize(screenWidth, screenHeight);
		
		float widthStretch = 1.0f / ((float)GetScreenWidth() / (float)GetScreenHeight());
		shaderFractal.SetWidthStretch(widthStretch);
	}

	UpdateFractalControls();
}

void UpdateFractalControls()
{
	float deltaTime = GetFrameTime();

	if (IsKeyPressed(KEY_R))
		ResetFractalParameters();

	UpdateFractalCamera();

	//Iteration keys
	if (IsKeyPressed(KEY_KP_ADD))
	{
		fractalParameters.maxIterations++;
		shaderFractal.SetMaxIterations(fractalParameters.maxIterations);
	}
	else if (IsKeyPressed(KEY_KP_SUBTRACT))
	{
		fractalParameters.maxIterations--;
		shaderFractal.SetMaxIterations(fractalParameters.maxIterations);
	}

	if (fractalParameters.type == FRACTAL_JULIA || fractalParameters.type == FRACTAL_MULTIBROT || fractalParameters.type == FRACTAL_MULTICORN)
	{
		//Power changing using keys
		if (IsKeyDown(KEY_F))
			fractalParameters.power -= 1.0f * deltaTime;
		else if (IsKeyDown(KEY_G))
			fractalParameters.power += 1.0f * deltaTime;

		//Round power down
		if (IsKeyPressed(KEY_H))
			fractalParameters.power = floor(fractalParameters.power);

		//Update shader fractal power if any of the above power keys were pressed
		if (IsKeyDown(KEY_F) || IsKeyDown(KEY_G) || IsKeyPressed(KEY_H))
			shaderFractal.SetPower(fractalParameters.power);
	}

	if (fractalParameters.type == FRACTAL_JULIA)
	{
		//c panning using keys
		float movementSpeed = IsKeyDown(KEY_LEFT_SHIFT) ? 0.05f : 0.01f;

		if (IsKeyDown(KEY_A))
			fractalParameters.c.x -= (movementSpeed * deltaTime) / fractalParameters.zoom;
		else if (IsKeyDown(KEY_D))
			fractalParameters.c.x += (movementSpeed * deltaTime) / fractalParameters.zoom;

		if (IsKeyDown(KEY_W))
			fractalParameters.c.y += (movementSpeed * deltaTime) / fractalParameters.zoom;
		else if (IsKeyDown(KEY_S))
			fractalParameters.c.y -= (movementSpeed * deltaTime) / fractalParameters.zoom;

		//c panning using mouse right click
		if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
		{
			Vector2 mouseDelta = GetMouseDelta();

			//fractal fits screen height
			fractalParameters.c.x += mouseDelta.x / (float)GetFractalRenderTextureHeight() / fractalParameters.zoom;
			fractalParameters.c.y -= mouseDelta.y / (float)GetFractalRenderTextureHeight() / fractalParameters.zoom;
		}

		//Update shader fractal c if necessary
		if (IsKeyDown(KEY_A) || IsKeyDown(KEY_D) || IsKeyDown(KEY_W) || IsKeyDown(KEY_S) || IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
			shaderFractal.SetC(fractalParameters.c);
	}
}

void UpdateFractalCamera()
{
	float deltaTime = GetFrameTime();

	//Camera panning using keys
	float movementSpeed = IsKeyDown(KEY_LEFT_SHIFT) ? 0.5f : 0.1f;

	if (IsKeyDown(KEY_LEFT))
		fractalParameters.position.x -= movementSpeed * deltaTime / fractalParameters.zoom;
	else if (IsKeyDown(KEY_RIGHT))
		fractalParameters.position.x += movementSpeed * deltaTime / fractalParameters.zoom;

	if (IsKeyDown(KEY_UP))
		fractalParameters.position.y += movementSpeed * deltaTime / fractalParameters.zoom;
	else if (IsKeyDown(KEY_DOWN))
		fractalParameters.position.y -= movementSpeed * deltaTime / fractalParameters.zoom;

	//Camera panning using mouse
	if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
	{
		Vector2 mouseDelta = GetMouseDelta();

		//fractal fits screen height
		fractalParameters.position.x -= mouseDelta.x / (float)GetFractalRenderTextureHeight() / fractalParameters.zoom;
		fractalParameters.position.y += mouseDelta.y / (float)GetFractalRenderTextureHeight() / fractalParameters.zoom;
	}

	//Update shader fractal position if necessary
	if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_UP) || IsKeyDown(KEY_DOWN) || IsMouseButtonDown(MOUSE_BUTTON_LEFT))
		shaderFractal.SetPosition(fractalParameters.position);

	//Camera zooming using keys
	zoomDeltaTime += deltaTime;

	if (IsKeyDown(KEY_I))
	{
		if (IsKeyPressed(KEY_I))
			zoomDeltaTime = 0.01f;

		while (zoomDeltaTime >= 0.01f)
		{
			fractalParameters.zoom *= 1.02f;
			zoomDeltaTime -= 0.01f;
		}
	}
	else if (IsKeyDown(KEY_O))
	{
		if (IsKeyPressed(KEY_O))
			zoomDeltaTime = 0.01f;

		while (zoomDeltaTime >= 0.01f)
		{
			fractalParameters.zoom /= 1.02f;
			zoomDeltaTime -= 0.01f;
		}
	}

	//Zooming using mouse wheel
	//up scroll: zoom in
	//down scroll: zoom out
	float mouseWheelMoved = GetMouseWheelMove();
	fractalParameters.zoom += fractalParameters.zoom * 0.1f * mouseWheelMoved;

	//Zooming using pinching
	Vector2 pinchMovement = GetGesturePinchVector();
	fractalParameters.zoom += fractalParameters.zoom * 0.5f * Vector2Length(pinchMovement);

	if (fractalParameters.zoom <= 1.0f / 1000.0f)
		fractalParameters.zoom = 1.0f / 1000.0f;

	//Update shader fractal zoom if necessary
	if (IsKeyDown(KEY_I) || IsKeyDown(KEY_O) || mouseWheelMoved != 0.0f || Vector2Length(pinchMovement) != 0.0f)
		shaderFractal.SetZoom(fractalParameters.zoom);
}
#pragma endregion

#pragma region UI function implementations

//Allows text with sub- and superscript
//Vector2 MeasureScriptTextEx(Font font, const char* text, float fontSize, float spacing);

//Allows text with sub- and superscript
//void DrawScriptTextEx(Font font, const char* text, Vector2 position, float fontSize, float spacing, Color tint);

float GetFontSizeForWidth(Font font, const char* text, float width, float spacingMultiplier)
{
	const float BASE_FONT_SIZE = 16.0f;
	return BASE_FONT_SIZE / MeasureTextEx(font, text, BASE_FONT_SIZE, BASE_FONT_SIZE * spacingMultiplier).x * width;
}

void DrawFractalGridAxises()
{
	const float GRID_LINE_THICKNESS = 2.0f;

	int screenWidth = GetScreenWidth();
	int screenHeight = GetScreenHeight();

	Vector2 fractalCenterScreenPosition = GetFractalToScreenPosition(Vector2Zero());

	//Vertical grid line is on screen
	if (fractalCenterScreenPosition.x >= 0.0f || fractalCenterScreenPosition.x <= (float)screenWidth)
	{
		DrawLineEx(Vector2{ fractalCenterScreenPosition.x, 0.0f }, Vector2{ fractalCenterScreenPosition.x, (float)screenHeight }, GRID_LINE_THICKNESS, WHITE);
	}

	//Horizontal grid line is on screen
	if (fractalCenterScreenPosition.y >= 0.0f || fractalCenterScreenPosition.y <= (float)screenHeight)
	{
		DrawLineEx(Vector2{ 0.0f, fractalCenterScreenPosition.y }, Vector2{ (float)screenWidth, fractalCenterScreenPosition.y }, GRID_LINE_THICKNESS, WHITE);
	}

	// Axis number markers

	//Bottom left of the screen has the smallest x & y
	Vector2 minFractalPosition = GetScreenToFractalPosition(Vector2{ 0.0f, (float)screenHeight});

	//Top right of the screen has the largest x & y
	Vector2 maxFractalPosition = GetScreenToFractalPosition(Vector2{(float)screenWidth, 0.0f});

	float markerLength = 20.0f;
	int numberFontSize = 18;
	float numberPadding = 5.0f;

	int zoomLevel = std::max(-(int)trunc(log10(fractalParameters.zoom)), -5);
	float increment = 0.5f * pow(10, (float)zoomLevel);

	std::string formatString = "%.0" + std::to_string(zoomLevel > 0 ? 0 : -zoomLevel + 1) + "f";

	//0 at center
	DrawText("0", fractalCenterScreenPosition.x + numberPadding, fractalCenterScreenPosition.y + numberPadding, numberFontSize, WHITE);

	//Start at multiple of increment closest to min fractal x and draw markers until max fractal x
	for (float x = minFractalPosition.x - fmod(minFractalPosition.x, increment); x <= maxFractalPosition.x + increment; x += increment)
	{
		//don't draw 0, due to floating point imprecision we can't check if x is equal to 0.0f, increment divided by 2.0f so the first increment after 0.0 is drawn
		if (abs(x) < increment / 2.0f)
			continue;

		Vector2 fractalScreenPosition = GetFractalToScreenPosition(Vector2{ x, 0.0f });

		DrawLineEx(Vector2{fractalScreenPosition.x, fractalCenterScreenPosition.y - markerLength / 2.0f}, Vector2{ fractalScreenPosition.x, fractalCenterScreenPosition.y + markerLength / 2.0f }, GRID_LINE_THICKNESS, WHITE);
		
		int numberLength = MeasureText(TextFormat(formatString.c_str(), x), numberFontSize);
		DrawText(TextFormat(formatString.c_str(), x), (int)fractalScreenPosition.x - numberLength / 2, (int)(fractalCenterScreenPosition.y + markerLength / 2.0f + numberPadding), numberFontSize, WHITE);
	}

	//Start at multiple of increment closest to min fractal y and draw markers until max fractal y
	for (float y = minFractalPosition.y - fmod(minFractalPosition.y, increment); y <= maxFractalPosition.y + increment; y += increment)
	{
		//don't draw 0, due to floating point imprecision we can't check if y is equal to 0.0f, increment divided by 2.0f so the first increment after 0.0 is drawn
		if (abs(y) < increment / 2.0f)
			continue;

		Vector2 fractalScreenPosition = GetFractalToScreenPosition(Vector2{ 0.0f, y });
		
		DrawLineEx(Vector2{ fractalCenterScreenPosition.x - markerLength / 2.0f, fractalScreenPosition.y }, Vector2{ fractalCenterScreenPosition.x + markerLength / 2.0f, fractalScreenPosition.y }, GRID_LINE_THICKNESS, WHITE);
	
		DrawText(TextFormat(formatString.c_str(), y), (int)(fractalCenterScreenPosition.x + markerLength / 2.0f + numberPadding), (int)fractalScreenPosition.y - numberFontSize / 2, numberFontSize, WHITE);
	}

	//Letters
	//Always visible
	//Clamped to screen edges
	//x leaves space in top right for y

	int letterFontSize = 24;
	int letterPadding = 10;

	DrawText("x >", screenWidth - MeasureText("x >", letterFontSize) - letterPadding, (int)std::clamp(fractalCenterScreenPosition.y - (float)letterFontSize - (float)letterPadding, (float)letterPadding * 2.0f + (float)letterFontSize, (float)screenHeight - (float)letterFontSize - (float)letterPadding), letterFontSize, WHITE);
	DrawText("y /\\", (int)std::clamp(fractalCenterScreenPosition.x + (float)letterPadding, (float)letterPadding, (float)screenWidth - (float)MeasureText("y /\\", letterFontSize) - (float)letterPadding), letterPadding, letterFontSize, WHITE);

}

void UpdateUI()
{
	
}

void DrawUI()
{
	int screenWidth = GetScreenWidth();
	int screenHeight = GetScreenHeight();

	if (showDebugInfo)
	{
		DrawFractalGridAxises();

		DrawFPS(10, 10);
		DrawText(TextFormat("Position : %f, %f", fractalParameters.position.x, fractalParameters.position.y), 10, 10 + 24, 24, GREEN);
		DrawText(TextFormat("Zoom: %f", fractalParameters.zoom), 10, 10 + 24 * 2, 24, GREEN);
		DrawText(TextFormat("Max iterations: %i", fractalParameters.maxIterations), 10, 10 + 24 * 3, 24, GREEN);
		DrawText(TextFormat("Screen size: %ix%i", screenWidth, screenHeight), 10, 10 + 24 * 4, 24, GREEN);
		DrawText(TextFormat("Color banding?: %s", (fractalParameters.colorBanding ? "Yes" : "Reduced")), 10, 10 + 24 * 5, 24, GREEN);
		DrawText(TextFormat("Pixels: %i", screenWidth * screenHeight), 10, 10 + 24 * 6, 24, GREEN);

		DrawText(GetFractalName(fractalParameters.type), 10, 10 + 24 * 7, 24, WHITE);

		if (fractalParameters.type == FRACTAL_JULIA || fractalParameters.type == FRACTAL_MULTIBROT || fractalParameters.type == FRACTAL_MULTICORN)
		{
			DrawText(TextFormat("Pow : %f", fractalParameters.power), 10, 10 + 24 * 8, 24, WHITE);
		}

		if (fractalParameters.type == FRACTAL_JULIA)
		{
			DrawText(TextFormat("c = %f + %f i", fractalParameters.c.x, fractalParameters.c.y), 10, 10 + 24 * 9, 24, WHITE);
		}
	}

	Font mainFont = GetFontDefault();
	Color mainBackgroundColor = DARKGRAY;
	const float FONT_SPACING_MULTIPLIER = 0.1f;

	float screenRatio = std::min((float)screenWidth / (float)DESIGN_WIDTH, (float)screenHeight / (float)DESIGN_HEIGHT);

	//Fractal selection panel
	{
		float fractalSelectionHeight = 32.0f * screenRatio;
		Rectangle fractalSelectionRect = Rectangle{ (float)screenWidth * 0.25f, (float)screenHeight - fractalSelectionHeight * 1.5f, (float)screenWidth * 0.5f, fractalSelectionHeight };

		DrawRectangleRec(fractalSelectionRect, ColorAlpha(mainBackgroundColor, 0.4f));

		const char* fractalName = GetFractalName(fractalParameters.type);
		float fractalNameFontSize = std::min(GetFontSizeForWidth(mainFont, fractalName, fractalSelectionRect.width * 0.8f, FONT_SPACING_MULTIPLIER), fractalSelectionHeight * 0.8f);
		Vector2 fractalNameTextSize = MeasureTextEx(mainFont, fractalName, fractalNameFontSize, fractalNameFontSize / 10.0f);
		DrawTextEx(mainFont, fractalName, Vector2{ fractalSelectionRect.x + fractalSelectionRect.width / 2.0f - fractalNameTextSize.x / 2.0f, fractalSelectionRect.y + fractalSelectionRect.height * 0.5f - fractalNameTextSize.y * 0.5f }, fractalNameFontSize, fractalNameFontSize / 10.0f, WHITE);
	}

}

#pragma endregion