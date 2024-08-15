﻿// Fractal Explorer.cpp : Defines the entry point for the application.
//

#include "Fractal Explorer.h"

#include <cmath>
#include <string>
#include <vector>
#include <filesystem>

#include "raylib.h"
#include "raymath.h"

#include "Resources.h"
#include "Fractal.h"
#include "ComplexNumbers/ComplexFloat.h"
#include "UI/UIUtils.h"
#include "UI/GridUtils.h"

const float FONT_SPACING_MULTIPLIER = 1.0f / 15.0f;

const char* fractalEquations[NUM_FRACTAL_TYPES] =
{
	"z ^ 2 + c",
	"(Re(z) - Im(z) i) ^ 2 + c",
	"(|Re(z) + |Im (z)| i) ^ 2 + c",
	"z ^ n + c",
	"z ^ n + c",
	"(Re(z) - Im(z) i) ^ n + c",
	"z - a * (P(z) / P'(z))"
};

FractalParameters fractalParameters = FractalParameters();
ShaderFractal shaderFractal;
bool showDebugInfo = false;

int zoomLevel = 0;
float gridIncrement = 5.0f;

//Delta times
float zoomDeltaTime = 0.0f;
float screenshotDeltaTime = 5.0f;

//Dots
bool isDraggingDot = false;
int draggingDotId = -1;

bool cursorOnUI = false;

bool flipYAxis = false;

void UpdateDrawFrame();

void Update();

#pragma region Fractal functions
Vector2 GetScreenToFractalPosition(Vector2 screenPosition);
Vector2 GetFractalToScreenPosition(Vector2 fractalPosition);

void ChangeFractal(FractalType fractalType);

void ResetFractalParameters();

void UpdateFractal();
void UpdateFractalControls();
void UpdateFractalCamera();

void TakeFractalScreenshot();
#pragma endregion

#pragma region UI functions
float GetFontSizeForWidth(Font font, const char* text, float width, float spacingMultiplier = 0.1f);

void DrawFractalGrid();

void UpdateZoomLevel();

//Immediate-Mode UI
void UpdateDrawUI();

void UpdateDrawFractalSelectionPanel();

void DrawDraggableDot(Vector2 position, float radius, Color fillColor, Color outlineColor, bool isHovered, bool isDown);

void UpdateDrawDraggableDots();
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

	LoadResources();

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
	UnloadResources();
	UnloadFractalRenderTexture();
	shaderFractal.Unload();

	CloseAudioDevice();
	CloseWindow();

	return 0;
}

void UpdateDrawFrame()
{
	Update();

	BeginDrawing();
	{
		ClearBackground(BLACK);

		shaderFractal.Draw(Rectangle{ 0.0f, 0.0f, (float)GetScreenWidth(), (float)GetScreenHeight()}, false, flipYAxis);

		UpdateDrawUI();
	}
	EndDrawing();
}

void Update()
{
	UpdateFractal();

	if (IsKeyPressed(KEY_SPACE))
		showDebugInfo = !showDebugInfo;

	if (IsKeyPressed(KEY_J))
		TakeFractalScreenshot();
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
		flipYAxis ? (fragTexCoord.y + normalizedScreenOffset.y) / fractalParameters.zoom + fractalParameters.position.y : (fragTexCoord.y + normalizedScreenOffset.y) / -fractalParameters.zoom + fractalParameters.position.y
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
		((fractalPosition.x - fractalParameters.position.x) * widthStretch * fractalParameters.zoom - normalizedScreenOffset.x)* (float)screenWidth,
		flipYAxis ? ((fractalPosition.y - fractalParameters.position.y) * fractalParameters.zoom - normalizedScreenOffset.y) * (float)screenHeight : (-(fractalPosition.y - fractalParameters.position.y) * fractalParameters.zoom - normalizedScreenOffset.y) * (float)screenHeight
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

	flipYAxis = fractalType == FRACTAL_BURNING_SHIP;
}

void ResetFractalParameters()
{
	//position
	fractalParameters.position = Vector2{ 0.0f, 0.0f };
	shaderFractal.SetPosition(fractalParameters.position);

	//zoom
	fractalParameters.zoom = 1.0f;
	shaderFractal.SetZoom(fractalParameters.zoom);
	UpdateZoomLevel();

	//max iterations
	fractalParameters.maxIterations = 300;
	shaderFractal.SetMaxIterations(fractalParameters.maxIterations);

	//c
	fractalParameters.c = Vector2{ 0.0f, 0.0f };

	if (shaderFractal.SupportsC())
		shaderFractal.SetC(fractalParameters.c);

	//powers
	if (fractalParameters.type == FRACTAL_MULTIBROT || fractalParameters.type == FRACTAL_MULTICORN)
		fractalParameters.power = 3.0f;
	else
		fractalParameters.power = 2.0f;

	if (shaderFractal.SupportsPower())
		shaderFractal.SetPower(fractalParameters.power);

	//roots
	if (fractalParameters.type == FRACTAL_NEWTON_3DEG)
	{
		//Default roots are the roots to most known Newton Fractal (P(z) = z^3 - 1)
		fractalParameters.roots[0] = Vector2{1.0f, 0.0f};
		fractalParameters.roots[1] = Vector2{ -0.5f, sqrt(3.0f) / 2.0f };
		fractalParameters.roots[2] = Vector2{ -0.5f, -sqrt(3.0f) / 2.0f };
		/*fractalParameters.roots[0] = Vector2{0.0f, 1.0f};
		fractalParameters.roots[1] = Vector2{ 0.0f, -1.0f };
		fractalParameters.roots[2] = Vector2{ -1.5f, 0.0f };*/
	}
	else if (fractalParameters.type == FRACTAL_NEWTON_4DEG)
	{
		//Default roots are the roots to P(z) = z^4 - 1
		fractalParameters.roots[0] = Vector2{ 1.0f, 0.0f };
		fractalParameters.roots[1] = Vector2{ -1.0f, 0.0f };
		fractalParameters.roots[2] = Vector2{ 0.0f, 1.0f };
		fractalParameters.roots[3] = Vector2{ 0.0f, -1.0f };
	}

	if (shaderFractal.GetNumSettableRoots() > 0)
		shaderFractal.SetRoots(fractalParameters.roots.data(), shaderFractal.GetNumSettableRoots());

	//a
	fractalParameters.a = Vector2{ 1.0f, 0.0f };

	if (shaderFractal.SupportsA())
		shaderFractal.SetA(fractalParameters.a);

	//Color banding

	fractalParameters.colorBanding = false;

	if (shaderFractal.SupportsColorBanding())
		shaderFractal.SetColorBanding(fractalParameters.colorBanding);
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
	else if (IsKeyPressed(KEY_SEVEN))
		ChangeFractal(FRACTAL_NEWTON_3DEG);

	if (IsKeyPressed(KEY_T))
		ChangeFractal((FractalType)(((int)fractalParameters.type + 1) % NUM_FRACTAL_TYPES));

	if (IsKeyPressed(KEY_E) && shaderFractal.SupportsColorBanding())
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

	if (shaderFractal.SupportsPower())
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
		fractalParameters.position.y += (flipYAxis ? -movementSpeed : movementSpeed) * deltaTime / fractalParameters.zoom;
	else if (IsKeyDown(KEY_DOWN))
		fractalParameters.position.y -= (flipYAxis ? -movementSpeed : movementSpeed) * deltaTime / fractalParameters.zoom;

	//Camera panning using mouse, if not on ui
	if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && !cursorOnUI)
	{
		Vector2 mouseDelta = GetMouseDelta();

		//fractal fits screen height
		fractalParameters.position.x -= mouseDelta.x / (float)GetFractalRenderTextureHeight() / fractalParameters.zoom;
		fractalParameters.position.y += (flipYAxis ? -mouseDelta.y : mouseDelta.y) / (float)GetFractalRenderTextureHeight() / fractalParameters.zoom;
	}

	//Update shader fractal position if necessary
	if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_UP) || IsKeyDown(KEY_DOWN) || (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && !isDraggingDot))
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

	//min & max zoom
	fractalParameters.zoom = std::clamp(fractalParameters.zoom, 1.0f / 100000.0f, pow(10.0f, 10.0f));

	//Update shader fractal zoom & zoomLevel if necessary
	if (IsKeyDown(KEY_I) || IsKeyDown(KEY_O) || mouseWheelMoved != 0.0f || Vector2Length(pinchMovement) != 0.0f)
	{
		shaderFractal.SetZoom(fractalParameters.zoom);
		UpdateZoomLevel();
	}
}

void TakeFractalScreenshot()
{
	//TODO: Web modifications

	//Fractal_Screenshots directory
	std::filesystem::path fractalScreenshotsPath = std::filesystem::current_path().append("Fractal_Screenshots");

	std::cout << "Checking if fractal screenshots directory exists..." << std::endl;
	if (!std::filesystem::exists(fractalScreenshotsPath))
	{
		bool success = std::filesystem::create_directories(fractalScreenshotsPath);

		if (!success)
		{
			std::cout << "Failed to create fractal screenshots directory!" << std::endl;
			return;
		}
		else
		{
			std::cout << "Created fractal screenshots directory!" << std::endl;
		}
	}
	else
	{
		std::cout << "Fractal screenshots directory exists!" << std::endl;
	}

	std::cout << fractalScreenshotsPath.string() << std::endl;

	//Get first unused screenshot number
	int num = 1;

	while (std::filesystem::exists(TextFormat("%s\\fractal_screenshot-%i.png", fractalScreenshotsPath.string().c_str(), num)))
		num++;

	Image fractalImage = shaderFractal.GenImage(false, flipYAxis);

	ExportImage(fractalImage, TextFormat("%s\\fractal_screenshot-%i.png", fractalScreenshotsPath.string().c_str(), num));
	
	UnloadImage(fractalImage);

	screenshotDeltaTime = 0.0f;

	//TODO: play screenshot sfx here
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

void DrawFractalGrid()
{
	const float GRID_LINE_THICKNESS = 2.0f;
	const float GRID_LINE_ALPHA = 0.25f;

	int screenWidth = GetScreenWidth();
	int screenHeight = GetScreenHeight();

	Font mainFontSemibold = GetFont("mainFontSemibold");

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

	//Bottom left of the screen has the smallest x & y (if y unflipped)
	Vector2 minFractalPosition = GetScreenToFractalPosition(Vector2{ 0.0f, (float)screenHeight});

	//Top right of the screen has the largest x & y (if y unflipped)
	Vector2 maxFractalPosition = GetScreenToFractalPosition(Vector2{(float)screenWidth, 0.0f});

	//switch min & max y if flipping y axis
	if (flipYAxis)
	{
		float tempY = minFractalPosition.y;
		minFractalPosition.y = maxFractalPosition.y;
		maxFractalPosition.y = tempY;
	}

	float markerLength = 20.0f;
	float numberFontSize = 20.0f;
	float numberPadding = 5.0f;

	std::string formatString = "%.0" + std::to_string(zoomLevel > 0 ? 0 : zoomLevel / -3) + "f";

	//0 at center
	DrawTextEx(mainFontSemibold, "0", Vector2{fractalCenterScreenPosition.x + numberPadding, fractalCenterScreenPosition.y + numberPadding}, numberFontSize, numberFontSize / 10.0f ,WHITE);

	//Start at multiple of increment closest to min fractal x and draw markers until max fractal x

	int numIncrementsX = (int)((GetClosestLargerMultipleOf(maxFractalPosition.x, gridIncrement) - GetClosestSmallerMultipleOf(minFractalPosition.x, gridIncrement)) / gridIncrement) + 2;

	for (int incrementX = 0; incrementX < numIncrementsX; incrementX++)
	{
		float x = GetClosestSmallerMultipleOf(minFractalPosition.x, gridIncrement) + gridIncrement * (incrementX - 1);

		//don't draw 0, due to floating point imprecision we can't check if x is equal to 0.0f, increment divided by 2.0f so the first increment after 0.0 is drawn
		if (abs(x) < gridIncrement / 2.0f)
			continue;

		Vector2 fractalScreenPosition = GetFractalToScreenPosition(Vector2{ x, 0.0f });

		DrawLineEx(Vector2{ fractalScreenPosition.x, 0.0f }, Vector2{ fractalScreenPosition.x, (float)screenHeight }, GRID_LINE_THICKNESS, ColorAlpha(WHITE, GRID_LINE_ALPHA));
		DrawLineEx(Vector2{fractalScreenPosition.x, fractalCenterScreenPosition.y - markerLength / 2.0f}, Vector2{ fractalScreenPosition.x, fractalCenterScreenPosition.y + markerLength / 2.0f }, GRID_LINE_THICKNESS, WHITE);
		
		float numberLength = MeasureTextEx(mainFontSemibold, TextFormat(formatString.c_str(), x), numberFontSize, numberFontSize * FONT_SPACING_MULTIPLIER).x;

		DrawTextEx(
			mainFontSemibold, 
			TextFormat(formatString.c_str(), x), 
			Vector2{ fractalScreenPosition.x - numberLength / 2.0f, fractalCenterScreenPosition.y + markerLength / 2.0f + numberPadding },
			numberFontSize, 
			numberFontSize * FONT_SPACING_MULTIPLIER, 
			WHITE
		);
	}

	//Start at multiple of increment closest to min fractal y and draw markers until max fractal y

	int numIncrementsY = (int)((GetClosestLargerMultipleOf(maxFractalPosition.y, gridIncrement) - GetClosestSmallerMultipleOf(minFractalPosition.y, gridIncrement)) / gridIncrement) + 2;

	for (int incrementY = 0; incrementY < numIncrementsY; incrementY++)
	{
		float y = GetClosestSmallerMultipleOf(minFractalPosition.y, gridIncrement) + gridIncrement * (incrementY - 1);

		//don't draw 0, due to floating point imprecision we can't check if y is equal to 0.0f, increment divided by 2.0f so the first increment after 0.0 is drawn
		if (abs(y) < gridIncrement / 2.0f)
			continue;

		Vector2 fractalScreenPosition = GetFractalToScreenPosition(Vector2{ 0.0f, y });
		
		DrawLineEx(Vector2{ 0.0f, fractalScreenPosition.y }, Vector2{ (float)screenWidth, fractalScreenPosition.y }, GRID_LINE_THICKNESS, ColorAlpha(WHITE, GRID_LINE_ALPHA));
		DrawLineEx(Vector2{ fractalCenterScreenPosition.x - markerLength / 2.0f, fractalScreenPosition.y }, Vector2{ fractalCenterScreenPosition.x + markerLength / 2.0f, fractalScreenPosition.y }, GRID_LINE_THICKNESS, WHITE);
	
		float numberLength = MeasureTextEx(mainFontSemibold, TextFormat((formatString + "i").c_str(), y), numberFontSize, numberFontSize * FONT_SPACING_MULTIPLIER).x;

		DrawTextEx(
			mainFontSemibold,
			TextFormat((formatString + "i").c_str(), y),
			Vector2{ fractalCenterScreenPosition.x - markerLength / 2.0f - numberPadding - numberLength, fractalScreenPosition.y - numberFontSize / 2.0f },
			numberFontSize,
			numberFontSize * FONT_SPACING_MULTIPLIER,
			WHITE
		);
	}

	//Letters
	//Always visible
	//Clamped to screen edges
	//x leaves space in top right for y

	float letterFontSize = 24.0f;
	float letterPadding = 10.0f;

	Vector2 letterXSize = MeasureTextEx(mainFontSemibold, "x >", letterFontSize, letterFontSize * FONT_SPACING_MULTIPLIER);
	DrawTextEx(
		mainFontSemibold,
		"x >",
		Vector2
		{
			screenWidth - letterXSize.x - letterPadding,
			std::clamp(
				fractalCenterScreenPosition.y - letterXSize.y - letterPadding,
				letterPadding * 2.0f + letterXSize.y,
				std::max((float)screenHeight - letterXSize.y - letterPadding, letterPadding * 2.0f + letterXSize.y)
			)
		},
		letterFontSize,
		letterFontSize * FONT_SPACING_MULTIPLIER,
		WHITE
	);

	Vector2 letterYSize = MeasureTextEx(mainFontSemibold, "y ^", letterFontSize, letterFontSize * FONT_SPACING_MULTIPLIER);
	DrawTextEx(
		mainFontSemibold,
		"y ^",
		Vector2
		{
			std::clamp(
				fractalCenterScreenPosition.x + letterPadding,
				letterPadding,
				std::max((float)screenWidth - letterYSize.x - letterPadding, letterPadding)
			),
			letterPadding
		},
		letterFontSize,
		letterFontSize * FONT_SPACING_MULTIPLIER,
		WHITE
	);

}

void UpdateZoomLevel()
{
	int screenWidth = GetScreenWidth();
	int screenHeight = GetScreenHeight();

	//Bottom left of the screen has the smallest x & y
	Vector2 minFractalPosition = GetScreenToFractalPosition(Vector2{ 0.0f, (float)screenHeight });

	//Top right of the screen has the largest x & y
	Vector2 maxFractalPosition = GetScreenToFractalPosition(Vector2{ (float)screenWidth, 0.0f });

	float minMaxDifference = std::max(maxFractalPosition.x - minFractalPosition.x, maxFractalPosition.y - minFractalPosition.y);

	int defaultIncrement = 5;

	zoomLevel = 0;
	gridIncrement = (float)defaultIncrement;

	if (gridIncrement >= minMaxDifference / 15.0f)
	{
		//Increment can become so small that if added to position, it adds 0 instead due to floating point imprecision
		while (gridIncrement >= minMaxDifference / 15.0f && minFractalPosition.x + gridIncrement != minFractalPosition.x && minFractalPosition.y + gridIncrement != minFractalPosition.y)
		{
			zoomLevel--;

			if (defaultIncrement == 1)
				defaultIncrement = 5;
			else
				defaultIncrement /= 2;

			gridIncrement = pow(10.0f, (float)(zoomLevel / 3)) * defaultIncrement;
		}

		//go back a level
		zoomLevel++;

		switch (defaultIncrement)
		{
			case 5:
				defaultIncrement = 1;
				break;
			case 2:
				defaultIncrement = 5;
				break;
			case 1:
				defaultIncrement = 2;
				break;
		}

		gridIncrement = pow(10.0f, (float)(zoomLevel / 3)) * defaultIncrement;
	}
	else
	{
		//Increment can so small that if added to position, it adds 0 instead due to floating point imprecision, keep increasing zoom level if that happens
		while (gridIncrement < minMaxDifference / 15.0f || maxFractalPosition.x + gridIncrement == maxFractalPosition.x || maxFractalPosition.y + gridIncrement == maxFractalPosition.y)
		{
			zoomLevel++;

			if (defaultIncrement == 20)
				defaultIncrement = 5;
			else
				defaultIncrement *= 2;

			gridIncrement = pow(10.0f, (float)(zoomLevel / 3)) * defaultIncrement;
		}
	}
}

void UpdateDrawUI()
{
	int screenWidth = GetScreenWidth();
	int screenHeight = GetScreenHeight();

	float deltaTime = GetFrameTime();

	cursorOnUI = false;

	if (showDebugInfo)
	{
		DrawFractalGrid();

		UpdateDrawDraggableDots();

		if (isDraggingDot)
			cursorOnUI = true;

		const int STAT_FONT_SIZE = 24;
		int statY = 10;

		DrawFPS(10, 10);
		statY += STAT_FONT_SIZE;

		DrawText(TextFormat("Position : %f, %f", fractalParameters.position.x, fractalParameters.position.y), 10, statY, STAT_FONT_SIZE, GREEN);
		statY += STAT_FONT_SIZE;

		DrawText(TextFormat("Zoom: %f", fractalParameters.zoom), 10, statY, STAT_FONT_SIZE, GREEN);
		statY += STAT_FONT_SIZE;

		DrawText(TextFormat("Max iterations: %i", fractalParameters.maxIterations), 10, statY, STAT_FONT_SIZE, GREEN);
		statY += STAT_FONT_SIZE;

		DrawText(TextFormat("Screen size: %ix%i", screenWidth, screenHeight), 10, statY, STAT_FONT_SIZE, GREEN);
		statY += STAT_FONT_SIZE;

		DrawText(TextFormat("Pixels: %i", screenWidth * screenHeight), 10, statY, STAT_FONT_SIZE, GREEN);
		statY += STAT_FONT_SIZE;

		DrawText(GetFractalName(fractalParameters.type), 10, statY, STAT_FONT_SIZE, WHITE);
		statY += STAT_FONT_SIZE;

		if (shaderFractal.SupportsPower())
		{
			DrawText(TextFormat("Pow : %f", fractalParameters.power), 10, statY, STAT_FONT_SIZE, WHITE);
			statY += STAT_FONT_SIZE;
		}

		if (shaderFractal.SupportsC())
		{
			DrawText(TextFormat("c = %f + %f i", fractalParameters.c.x, fractalParameters.c.y), 10, statY, STAT_FONT_SIZE, WHITE);
			statY += STAT_FONT_SIZE;
		}

		if (shaderFractal.SupportsColorBanding())
		{
			DrawText(TextFormat("Color banding?: %s", (fractalParameters.colorBanding ? "Yes" : "Reduced")), 10, statY, STAT_FONT_SIZE, WHITE);
			statY += STAT_FONT_SIZE;
		}

		if (shaderFractal.SupportsA())
		{
			DrawText(TextFormat("a = %f + %f i", fractalParameters.a.x, fractalParameters.a.y), 10, statY, STAT_FONT_SIZE, WHITE);
			statY += STAT_FONT_SIZE;
		}

		for (int i = 0; i < shaderFractal.GetNumSettableRoots(); i++)
		{
			DrawText(TextFormat("r%i = %f + %f i", i + 1, fractalParameters.roots[i].x, fractalParameters.roots[i].y), 10, statY, STAT_FONT_SIZE, WHITE);
			statY += STAT_FONT_SIZE;
		}

		if (fractalParameters.type == FRACTAL_NEWTON_3DEG)
		{
			ComplexFloat root1 = ComplexFloat(fractalParameters.roots[0].x, fractalParameters.roots[0].y);
			ComplexFloat root2 = ComplexFloat(fractalParameters.roots[1].x, fractalParameters.roots[1].y);
			ComplexFloat root3 = ComplexFloat(fractalParameters.roots[2].x, fractalParameters.roots[2].y);

			//P(z)
			{
				ComplexFloat secondDegreeFactor = -root1 - root2 - root3;
				ComplexFloat firstDegreeFactor = (root1 * root2) + (root2 * root3) + (root1 * root3);
				ComplexFloat constant = -(root1 * root2 * root3);

				DrawText(TextFormat("P(z) ~= z^3 + (%.02f + %.02f i)z^2 + (%.02f + %.02f i)z + (%.02f + %.02f i)", secondDegreeFactor.real, secondDegreeFactor.imaginary, firstDegreeFactor.real, firstDegreeFactor.imaginary, constant.real, constant.imaginary), 10, statY, STAT_FONT_SIZE, WHITE);
				statY += STAT_FONT_SIZE;
			}
		}
		else if (fractalParameters.type == FRACTAL_NEWTON_4DEG)
		{
			ComplexFloat root1 = ComplexFloat(fractalParameters.roots[0].x, fractalParameters.roots[0].y);
			ComplexFloat root2 = ComplexFloat(fractalParameters.roots[1].x, fractalParameters.roots[1].y);
			ComplexFloat root3 = ComplexFloat(fractalParameters.roots[2].x, fractalParameters.roots[2].y);
			ComplexFloat root4 = ComplexFloat(fractalParameters.roots[3].x, fractalParameters.roots[3].y);

			//P(z)
			{
				ComplexFloat thirdDegreeFactor = -root1 - root2 - root3 - root4;
				ComplexFloat secondDegreeFactor = root1 * (root2 + root3 + root4) + root2 * (root3 + root4) + root3 * root4;
				ComplexFloat firstDegreeFactor = -root3 * root4 * (root1 + root2) - root1 * root2 * (root3 + root4);
				ComplexFloat constant = root1 * root2 * root3 * root4;

				DrawText(TextFormat("P(z) ~= z^4 + (%.02f + %.02f i)z^3 + (%.02f + %.02f i)z^2 + (%.02f + %.02f i)z + (%.02f + %.02f i)", thirdDegreeFactor.real, thirdDegreeFactor.imaginary, secondDegreeFactor.real, secondDegreeFactor.imaginary, firstDegreeFactor.real, firstDegreeFactor.imaginary, constant.real, constant.imaginary), 10, statY, STAT_FONT_SIZE, WHITE);
				statY += STAT_FONT_SIZE;
			}
		}
	}

	UpdateDrawFractalSelectionPanel();

	//Screenshot flash
	screenshotDeltaTime += deltaTime;

	if (screenshotDeltaTime <= 0.5f)
		DrawRectangle(0, 0, screenWidth, screenHeight, ColorAlpha(WHITE, 1.0f - (screenshotDeltaTime / 0.5f) * (screenshotDeltaTime / 0.5f)));
}

void UpdateDrawFractalSelectionPanel()
{
	int screenWidth = GetScreenWidth();
	int screenHeight = GetScreenHeight();

	Font mainFontSemibold = GetFont("mainFontSemibold");
	Color mainBackgroundColor = DARKGRAY;

	float screenRatio = std::min((float)screenWidth / (float)DESIGN_WIDTH, (float)screenHeight / (float)DESIGN_HEIGHT);

	//Main rect

	float fractalSelectionHeight = 32.0f * screenRatio;
	Rectangle fractalSelectionRect = Rectangle{ (float)screenWidth * 0.25f, (float)screenHeight - fractalSelectionHeight * 1.5f, (float)screenWidth * 0.5f, fractalSelectionHeight };

	DrawRectangleRec(Rectangle{ fractalSelectionRect.x + fractalSelectionRect.height, fractalSelectionRect.y, fractalSelectionRect.width - fractalSelectionRect.height * 2.0f, fractalSelectionRect.height }, ColorAlpha(mainBackgroundColor, 0.4f));

	if (IsRectangleHovered(fractalSelectionRect))
		cursorOnUI = true;

	//Fractal name

	const char* fractalName = GetFractalName(fractalParameters.type);
	float fractalNameFontSize = std::min(GetFontSizeForWidth(mainFontSemibold, fractalName, (fractalSelectionRect.width - fractalSelectionRect.height * 2.0f) * 0.8f, FONT_SPACING_MULTIPLIER), fractalSelectionHeight * 0.8f);
	Vector2 fractalNameTextSize = MeasureTextEx(mainFontSemibold, fractalName, fractalNameFontSize, fractalNameFontSize * FONT_SPACING_MULTIPLIER);
	DrawTextEx(mainFontSemibold, fractalName, Vector2{ fractalSelectionRect.x + fractalSelectionRect.width / 2.0f - fractalNameTextSize.x / 2.0f, fractalSelectionRect.y + fractalSelectionRect.height * 0.5f - fractalNameTextSize.y * 0.5f }, fractalNameFontSize, fractalNameFontSize * FONT_SPACING_MULTIPLIER, WHITE);

	//Previous fractal button

	Rectangle prevButtonRect = Rectangle{ fractalSelectionRect.x, fractalSelectionRect.y, fractalSelectionRect.height, fractalSelectionRect.height };
	Color prevButtonColor = mainBackgroundColor;

	if (IsRectangleHovered(prevButtonRect) && IsMouseButtonDown(MOUSE_BUTTON_LEFT))
		prevButtonColor = RAYWHITE;
	else if (IsRectangleHovered(prevButtonRect))
		prevButtonColor = LIGHTGRAY;

	DrawRectangleRec(prevButtonRect, ColorAlpha(prevButtonColor, 0.6f));

	float prevButtonFontSize = std::min(GetFontSizeForWidth(mainFontSemibold, "<", prevButtonRect.width * 0.8f, FONT_SPACING_MULTIPLIER), prevButtonRect.height * 0.8f);
	Vector2 prevButtonTextSize = MeasureTextEx(mainFontSemibold, "<", prevButtonFontSize, prevButtonFontSize * FONT_SPACING_MULTIPLIER);
	DrawTextEx(mainFontSemibold, "<", Vector2{ prevButtonRect.x + (prevButtonRect.width - prevButtonTextSize.x) * 0.5f, prevButtonRect.y + (prevButtonRect.height - prevButtonTextSize.y) * 0.5f }, prevButtonFontSize, prevButtonFontSize * FONT_SPACING_MULTIPLIER, WHITE);

	if (IsRectanglePressed(prevButtonRect))
		ChangeFractal((FractalType)(Wrap((int)fractalParameters.type - 1, 0, NUM_FRACTAL_TYPES)));

	//Next fractal button

	Rectangle nextButtonRect = Rectangle{ fractalSelectionRect.x + fractalSelectionRect.width - fractalSelectionRect.height, fractalSelectionRect.y, fractalSelectionRect.height, fractalSelectionRect.height };
	Color nextButtonColor = mainBackgroundColor;

	if (IsRectangleHovered(nextButtonRect) && IsMouseButtonDown(MOUSE_BUTTON_LEFT))
		nextButtonColor = RAYWHITE;
	else if (IsRectangleHovered(nextButtonRect))
		nextButtonColor = LIGHTGRAY;

	DrawRectangleRec(nextButtonRect, ColorAlpha(nextButtonColor, 0.6f));

	float nextButtonFontSize = std::min(GetFontSizeForWidth(mainFontSemibold, ">", nextButtonRect.width * 0.8f, FONT_SPACING_MULTIPLIER), nextButtonRect.height * 0.8f);
	Vector2 nextButtonTextSize = MeasureTextEx(mainFontSemibold, ">", nextButtonFontSize, nextButtonFontSize * FONT_SPACING_MULTIPLIER);
	DrawTextEx(mainFontSemibold, ">", Vector2{ nextButtonRect.x + (nextButtonRect.width - nextButtonTextSize.x) * 0.5f, nextButtonRect.y + (nextButtonRect.height - nextButtonTextSize.y) * 0.5f }, nextButtonFontSize, nextButtonFontSize * FONT_SPACING_MULTIPLIER, WHITE);

	if (IsRectanglePressed(nextButtonRect))
		ChangeFractal((FractalType)(((int)fractalParameters.type + 1) % NUM_FRACTAL_TYPES));

	//Top rect
	Rectangle buttonPanelRect = Rectangle{ fractalSelectionRect.x + fractalSelectionRect.height, fractalSelectionRect.y - fractalSelectionRect.height, fractalSelectionRect.width - fractalSelectionRect.height * 2.0f, fractalSelectionRect.height };

	DrawRectangleRec(buttonPanelRect, ColorAlpha(mainBackgroundColor, 0.2f));

	if (IsRectangleHovered(buttonPanelRect))
		cursorOnUI = true;

	int buttonIndex = 0;

	//Screenshot button

	Rectangle screenshotButtonRect = Rectangle{ buttonPanelRect.x + buttonPanelRect.height * buttonIndex, buttonPanelRect.y, buttonPanelRect.height, buttonPanelRect.height };
	
	DrawTextureButton(GetTexture("icon_screenshot"), screenshotButtonRect, WHITE, LIGHTGRAY, DARKGRAY);

	buttonIndex++;

	if (IsRectanglePressed(screenshotButtonRect))
		TakeFractalScreenshot();

	//Color banding button (if supported)
	if (shaderFractal.SupportsColorBanding())
	{
		Rectangle colorBandingButtonRect = Rectangle{ buttonPanelRect.x + buttonPanelRect.height * buttonIndex, buttonPanelRect.y, buttonPanelRect.height, buttonPanelRect.height };
		
		DrawTextureButton(fractalParameters.colorBanding ? GetTexture("icon_colorbanding_on") : GetTexture("icon_colorbanding_off"), colorBandingButtonRect, WHITE, LIGHTGRAY, DARKGRAY);

		buttonIndex++;

		if (IsRectanglePressed(colorBandingButtonRect))
		{
			fractalParameters.colorBanding = !fractalParameters.colorBanding;
			shaderFractal.SetColorBanding(fractalParameters.colorBanding);
		}
	}

	//Power buttons (if supported)
	if (shaderFractal.SupportsPower())
	{
		//Subtract
		Rectangle powerSubtractButtonRect = Rectangle{ buttonPanelRect.x + buttonPanelRect.height * buttonIndex, buttonPanelRect.y, buttonPanelRect.height, buttonPanelRect.height };

		DrawTextureButton(GetTexture("icon_power_subtract"), powerSubtractButtonRect, WHITE, LIGHTGRAY, DARKGRAY);

		buttonIndex++;

		if (IsRectangleHovered(powerSubtractButtonRect) && IsMouseButtonDown(MOUSE_BUTTON_LEFT))
		{
			fractalParameters.power -= GetFrameTime();
			shaderFractal.SetPower(fractalParameters.power);
		}

		//Add
		Rectangle powerAddButtonRect = Rectangle{ buttonPanelRect.x + buttonPanelRect.height * buttonIndex, buttonPanelRect.y, buttonPanelRect.height, buttonPanelRect.height };

		DrawTextureButton(GetTexture("icon_power_add"), powerAddButtonRect, WHITE, LIGHTGRAY, DARKGRAY);

		buttonIndex++;

		if (IsRectangleHovered(powerAddButtonRect) && IsMouseButtonDown(MOUSE_BUTTON_LEFT))
		{
			fractalParameters.power += GetFrameTime();
			shaderFractal.SetPower(fractalParameters.power);
		}

		//Floor
		Rectangle powerFloorButtonRect = Rectangle{ buttonPanelRect.x + buttonPanelRect.height * buttonIndex, buttonPanelRect.y, buttonPanelRect.height, buttonPanelRect.height };

		DrawTextureButton(GetTexture("icon_power_floor"), powerFloorButtonRect, WHITE, LIGHTGRAY, DARKGRAY);

		buttonIndex++;

		if (IsRectanglePressed(powerFloorButtonRect))
		{
			fractalParameters.power = floor(fractalParameters.power);
			shaderFractal.SetPower(fractalParameters.power);
		}
	}
}

void DrawDraggableDot(Vector2 position, float radius, Color fillColor, Color outlineColor, bool isHovered, bool isDown)
{
	DrawCircleV(position, radius, outlineColor);
	DrawCircleV(position, radius * 2.0f / 3.0f, isHovered ? ColorBrightness(fillColor, isDown ? -0.6f : -0.3f) : fillColor);
}

void UpdateDrawDraggableDots()
{
	const float DRAGGABLE_DOT_RADIUS = 6.0f;

	const float SNAP_PIXELS = 8.0f;

	const float LABEL_FONT_SIZE = 32.0f;
	const Vector2 LABEL_OFFSET = Vector2{ 4.0f, -4.0f };

	Font& mainFontSemibold = GetFont("mainFontSemibold");

	if (shaderFractal.SupportsC())
	{
		Vector2 cScreenPosition = GetFractalToScreenPosition(fractalParameters.c);

		bool dotPressed = IsCirclePressed(cScreenPosition, DRAGGABLE_DOT_RADIUS);

		//Start drag
		if (dotPressed && !isDraggingDot)
		{
			isDraggingDot = true;
			draggingDotId = 0;
		}

		//Update drag
		if (draggingDotId == 0 && isDraggingDot)
		{
			cScreenPosition = GetMousePosition();

			Vector2 newC = GetScreenToFractalPosition(cScreenPosition);
			Vector2 snapC = SnapTo2DGrid(newC, gridIncrement);

			//If close to grid intersection, snap to it
			Vector2 screenSnapC = GetFractalToScreenPosition(snapC);
			Vector2 screenDifC = Vector2Subtract(screenSnapC, cScreenPosition);

			if (abs(screenDifC.x) < SNAP_PIXELS)
			{
				newC.x = snapC.x;
				cScreenPosition.x = screenSnapC.x;
			}

			if (abs(screenDifC.y) < SNAP_PIXELS)
			{
				newC.y = snapC.y;
				cScreenPosition.y = screenSnapC.y;
			}

			fractalParameters.c = newC;
			shaderFractal.SetC(fractalParameters.c);
		}

		DrawDraggableDot(cScreenPosition, DRAGGABLE_DOT_RADIUS, WHITE, BLACK, IsCircleHovered(cScreenPosition, 6.0f), draggingDotId == 0);
		DrawTextEx(mainFontSemibold, "c", Vector2Add(Vector2{ cScreenPosition.x, cScreenPosition.y - LABEL_FONT_SIZE }, LABEL_OFFSET), LABEL_FONT_SIZE, LABEL_FONT_SIZE * FONT_SPACING_MULTIPLIER, WHITE);
	}

	if (shaderFractal.SupportsA())
	{
		Vector2 aScreenPosition = GetFractalToScreenPosition(fractalParameters.a);

		bool dotPressed = IsCirclePressed(aScreenPosition, DRAGGABLE_DOT_RADIUS);

		//Start drag
		if (dotPressed && !isDraggingDot)
		{
			isDraggingDot = true;
			draggingDotId = 1;
		}

		//Update drag
		if (draggingDotId == 1 && isDraggingDot)
		{
			aScreenPosition = GetMousePosition();

			Vector2 newA = GetScreenToFractalPosition(aScreenPosition);
			Vector2 snapA = SnapTo2DGrid(newA, gridIncrement);

			//If close to grid intersection, snap to it
			Vector2 screenSnapA = GetFractalToScreenPosition(snapA);
			Vector2 screenDifA = Vector2Subtract(screenSnapA, aScreenPosition);

			if (abs(screenDifA.x) < SNAP_PIXELS)
			{
				newA.x = snapA.x;
				aScreenPosition.x = screenSnapA.x;
			}

			if (abs(screenDifA.y) < SNAP_PIXELS)
			{
				newA.y = snapA.y;
				aScreenPosition.y = screenSnapA.y;
			}

			fractalParameters.a = newA;
			shaderFractal.SetA(fractalParameters.a);
		}

		DrawDraggableDot(aScreenPosition, DRAGGABLE_DOT_RADIUS, PINK, BLACK, IsCircleHovered(aScreenPosition, 6.0f), draggingDotId == 1);
		DrawTextEx(mainFontSemibold, "a", Vector2Add(Vector2{ aScreenPosition.x, aScreenPosition.y - LABEL_FONT_SIZE }, LABEL_OFFSET), LABEL_FONT_SIZE, LABEL_FONT_SIZE * FONT_SPACING_MULTIPLIER, WHITE);
	}

	for (int i = 0; i < shaderFractal.GetNumSettableRoots(); i++)
	{
		int id = 1 + i + 1;

		Vector2 rootScreenPosition = GetFractalToScreenPosition(fractalParameters.roots[i]);

		bool dotPressed = IsCirclePressed(rootScreenPosition, DRAGGABLE_DOT_RADIUS);

		//Start drag
		if (dotPressed && !isDraggingDot)
		{
			isDraggingDot = true;
			draggingDotId = id;
		}

		//Update drag
		if (draggingDotId == id && isDraggingDot)
		{
			rootScreenPosition = GetMousePosition();

			Vector2 newRoot = GetScreenToFractalPosition(rootScreenPosition);
			Vector2 snapRoot = SnapTo2DGrid(newRoot, gridIncrement);

			//If close to grid intersection, snap to it
			Vector2 screenSnapRoot = GetFractalToScreenPosition(snapRoot);
			Vector2 screenDifRoot = Vector2Subtract(screenSnapRoot, rootScreenPosition);

			if (abs(screenDifRoot.x) < SNAP_PIXELS)
			{
				newRoot.x = snapRoot.x;
				rootScreenPosition.x = screenSnapRoot.x;
			}

			if (abs(screenDifRoot.y) < SNAP_PIXELS)
			{
				newRoot.y = snapRoot.y;
				rootScreenPosition.y = screenSnapRoot.y;
			}

			fractalParameters.roots[i] = newRoot;
			shaderFractal.SetRoots(fractalParameters.roots.data(), shaderFractal.GetNumSettableRoots());
		}

		DrawDraggableDot(rootScreenPosition, DRAGGABLE_DOT_RADIUS, ColorFromHSV(120.0f * i, 1.0f, 1.0f), BLACK, IsCircleHovered(rootScreenPosition, 6.0f), draggingDotId == id);
		DrawTextEx(mainFontSemibold, TextFormat("r%i", i + 1), Vector2Add(Vector2{ rootScreenPosition.x, rootScreenPosition.y - LABEL_FONT_SIZE }, LABEL_OFFSET), LABEL_FONT_SIZE, LABEL_FONT_SIZE * FONT_SPACING_MULTIPLIER, WHITE);
	}

	//End drag
	if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
	{
		isDraggingDot = false;
		draggingDotId = -1;
	}
}

#pragma endregion