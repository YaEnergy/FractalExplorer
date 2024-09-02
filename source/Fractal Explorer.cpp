#include "Fractal Explorer.h"

#include <iostream>
#include <cmath>
#include <string>
#include <vector>
#include <filesystem>

#ifdef PLATFORM_WEB
#include <emscripten/emscripten.h>
#endif

#include "raylib.h"
#include "raymath.h"

#include "Resources.h"
#include "Fractal.h"
#include "ComplexNumbers/ComplexFloat.h"
#include "ComplexNumbers/ComplexPolynomial.h"
#include "UI/UIUtils.h"
#include "UI/GridUtils.h"
#include "UI/Notification.h"

namespace Explorer
{
	const float FONT_SPACING_MULTIPLIER = 1.0f / 15.0f;

	FractalParameters fractalParameters = FractalParameters();
	ShaderFractal shaderFractal;

	//Delta times

	float zoomDeltaTime = 0.0f;
	float screenshotDeltaTime = 5.0f;

	//Dots

	bool isDraggingDot = false;
	int draggingDotId = -1;

	//UI

	bool cursorOnUI = false;

	//true if a currently active press started on the ui
	bool activePressStartedOnUI = false;

	bool flipYAxis = false;
	bool showGrid = true;
	bool showInfoPanel = false;

	Notification notificationCurrent;

	float gridIncrement = 5.0f;
	
	bool showDebugInfo = false;

	float warningEndDeltaTime = 0.0f;
	bool warningEnded = false;

	void Update();

	#pragma region Fractal functions
	void ChangeFractal(FractalType fractalType);

	void ResetFractalParameters();

	void UpdateFractal();
	void UpdateFractalControls();
	void UpdateFractalCamera();

	void TakeFractalScreenshot();
	#pragma endregion

	#pragma region UI functions

	Vector2 GetSelectedMouseFractalPosition(float snapWithinPixels);

	bool CanHoldButton();

	void DrawFractalGrid();

	void UpdateGridIncrement();

	//Immediate-Mode UI
	void UpdateDrawUI();

	void UpdateDrawFractalSelectionPanel();

	void DrawFractalEquation();

	void DrawInfoPanel();

	void DrawStatInfo(const char* text, Vector2 position, Vector2 textPadding, float fontSize, Color textColor, Color backgroundColor);

	void DrawDraggableDot(Vector2 position, float radius, Color fillColor, Color outlineColor, bool isHovered, bool isDown);

	void UpdateDrawDraggableDots();

	void UpdateDrawNotification();

	void UpdateDrawWarning();

	//Sets cursorOnUI to true if rect is hovered, and activePressStartedOnUi if pressed
	void CursorPanelCheck(Rectangle rect);
	#pragma endregion

	void Init()
	{
		int screenWidth = GetScreenWidth();
		int screenHeight = GetScreenHeight();

		//Fractal set up
		InitFractalRenderTexture(screenWidth, screenHeight);

		fractalParameters.type = FRACTAL_MULTIBROT;
		shaderFractal = LoadShaderFractal(FRACTAL_MULTIBROT);

		shaderFractal.SetNormalizedCenterOffset(fractalParameters.normalizedCenterOffset);
		float widthStretch = GetWidthStretchForSize((float)screenWidth, (float)screenHeight);
		shaderFractal.SetWidthStretch(widthStretch);

		ResetFractalParameters();
	}

	void Deinit()
	{
		UnloadFractalRenderTexture();
		shaderFractal.Unload();
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
		if (!warningEnded)
			return;

		UpdateFractal();

		if (IsKeyPressed(KEY_SPACE))
			showDebugInfo = !showDebugInfo;

		if (IsKeyPressed(KEY_J))
			TakeFractalScreenshot();
	}

	#pragma region Fractal function implementations
	void ChangeFractal(FractalType fractalType)
	{
		fractalParameters.type = fractalType;

		shaderFractal.Unload();
		shaderFractal = LoadShaderFractal(fractalType);

		shaderFractal.SetNormalizedCenterOffset(fractalParameters.normalizedCenterOffset);
		float widthStretch = GetWidthStretchForSize((float)GetScreenWidth(), (float)GetScreenHeight());
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
		fractalParameters.zoom = 0.5f;
		shaderFractal.SetZoom(fractalParameters.zoom);
		UpdateGridIncrement();

		//Max iterations

#ifdef PLATFORM_WEB
		fractalParameters.maxIterations = 128;
#else //Desktop
		fractalParameters.maxIterations = 256;
#endif

		shaderFractal.SetMaxIterations(fractalParameters.maxIterations);

		//c
		fractalParameters.c = Vector2{ 0.0f, 0.0f };

		if (FractalSupportsC(fractalParameters.type))
			shaderFractal.SetC(fractalParameters.c);

		//powers
		fractalParameters.power = 2.0f;

		if (FractalSupportsPower(fractalParameters.type))
			shaderFractal.SetPower(fractalParameters.power);

		//roots
		int numRoots = GetFractalNumRoots(fractalParameters.type);

		if (numRoots == 2)
		{
			//Defaults roots are the roots to P(z) = z^2 - 1
			fractalParameters.roots[0] = Vector2{ 1.0f, 0.0f };
			fractalParameters.roots[1] = Vector2{ -1.0f, 0.0f };
		}
		else if (numRoots == 3)
		{
			//Default roots are the roots to P(z) = z^3 - 1
			fractalParameters.roots[0] = Vector2{1.0f, 0.0f};
			fractalParameters.roots[1] = Vector2{ -0.5f, sqrt(3.0f) / 2.0f };
			fractalParameters.roots[2] = Vector2{ -0.5f, -sqrt(3.0f) / 2.0f };
		}
		else if (numRoots == 4)
		{
			//Default roots are the roots to P(z) = z^4 - 1
			fractalParameters.roots[0] = Vector2{ 1.0f, 0.0f };
			fractalParameters.roots[1] = Vector2{ -1.0f, 0.0f };
			fractalParameters.roots[2] = Vector2{ 0.0f, 1.0f };
			fractalParameters.roots[3] = Vector2{ 0.0f, -1.0f };
		}
		else if (numRoots == 5)
		{
			//Default roots are the roots to P(z) = z^5 - 1
			fractalParameters.roots[0] = Vector2{ 1.0f, 0.0f };
			fractalParameters.roots[1] = Vector2{ (-1.0f + sqrt(5.0f)) / 4.0f, sqrt(10.0f + 2 * sqrt(5.0f)) / 4.0f };
			fractalParameters.roots[2] = Vector2{ (-1.0f + sqrt(5.0f)) / 4.0f, -sqrt(10.0f + 2 * sqrt(5.0f)) / 4.0f };
			fractalParameters.roots[3] = Vector2{ (-1.0f - sqrt(5.0f)) / 4.0f, sqrt(10.0f - 2 * sqrt(5.0f)) / 4.0f };
			fractalParameters.roots[4] = Vector2{ (-1.0f - sqrt(5.0f)) / 4.0f, -sqrt(10.0f - 2 * sqrt(5.0f)) / 4.0f };
		}

		if (numRoots > 0)
			shaderFractal.SetRoots(fractalParameters.roots.data(), numRoots);

		//a
		fractalParameters.a = Vector2{ 1.0f, 0.0f };

		if (FractalSupportsA(fractalParameters.type))
			shaderFractal.SetA(fractalParameters.a);

		//Color banding

		fractalParameters.colorBanding = false;

		if (FractalSupportsColorBanding(fractalParameters.type))
			shaderFractal.SetColorBanding(fractalParameters.colorBanding);
	}

	void UpdateFractal()
	{
		int screenWidth = GetScreenWidth();
		int screenHeight = GetScreenHeight();

		if (IsKeyPressed(KEY_ONE))
			ChangeFractal(FRACTAL_MULTIBROT);
		else if (IsKeyPressed(KEY_TWO))
			ChangeFractal(FRACTAL_MULTICORN);
		else if (IsKeyPressed(KEY_THREE))
			ChangeFractal(FRACTAL_BURNING_SHIP);
		else if (IsKeyPressed(KEY_FOUR))
			ChangeFractal(FRACTAL_JULIA);
		else if (IsKeyPressed(KEY_FIVE))
			ChangeFractal(FRACTAL_NEWTON_3DEG);
		else if (IsKeyPressed(KEY_SIX))
			ChangeFractal(FRACTAL_NEWTON_4DEG);
		else if (IsKeyPressed(KEY_SEVEN))
			ChangeFractal(FRACTAL_NEWTON_5DEG);
		else if (IsKeyPressed(KEY_EIGHT))
			ChangeFractal(FRACTAL_POLYNOMIAL_2DEG);
		else if (IsKeyPressed(KEY_NINE))
			ChangeFractal(FRACTAL_POLYNOMIAL_3DEG);

		if (IsKeyPressed(KEY_T))
			ChangeFractal((FractalType)(((int)fractalParameters.type + 1) % NUM_FRACTAL_TYPES));

		if (IsKeyPressed(KEY_E) && FractalSupportsColorBanding(fractalParameters.type))
		{
			fractalParameters.colorBanding = !fractalParameters.colorBanding;
			shaderFractal.SetColorBanding(fractalParameters.colorBanding);
		}

		//Update fractal render texture if screen size has changed
		if (screenWidth != GetFractalRenderTextureWidth() || screenHeight != GetFractalRenderTextureHeight())
		{
			SetFractalRenderTextureSize(screenWidth, screenHeight);
		
			float widthStretch = GetWidthStretchForSize((float)screenWidth, (float)screenHeight);
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
		if (IsKeyPressed(KEY_KP_ADD) || IsKeyPressed(KEY_L))
		{
			fractalParameters.maxIterations++;

#ifdef PLATFORM_WEB
			//Limit max iterations to 300 on web
			if (fractalParameters.maxIterations > 300)
			{
				fractalParameters.maxIterations = 300;
				notificationCurrent = Notification{ "Reached iteration limit on web version, use the desktop version to add more. (300)", 5.0f, WHITE };
			}
#endif
			
			shaderFractal.SetMaxIterations(fractalParameters.maxIterations);
		}
		else if ((IsKeyPressed(KEY_KP_SUBTRACT) || IsKeyPressed(KEY_K)) && fractalParameters.maxIterations > 0)
		{
			fractalParameters.maxIterations--;
			shaderFractal.SetMaxIterations(fractalParameters.maxIterations);
		}

		if (FractalSupportsPower(fractalParameters.type))
		{
			//Power changing using keys
			if (IsKeyDown(KEY_F))
				fractalParameters.power -= 1.0f * deltaTime;
			else if (IsKeyDown(KEY_G))
				fractalParameters.power += 1.0f * deltaTime;

			//Round power down
			if (IsKeyPressed(KEY_H))
				fractalParameters.power = floor(fractalParameters.power);

			if (fractalParameters.power < 0.0f)
				fractalParameters.power = 0.0f;

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
		if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && !activePressStartedOnUI)
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
		fractalParameters.zoom += fractalParameters.zoom * 0.25f * Vector2Length(pinchMovement);

		//min & max zoom
		fractalParameters.zoom = std::clamp(fractalParameters.zoom, 1.0f / 100000.0f, pow(10.0f, 10.0f));

		//Update shader fractal zoom & grid increment if necessary
		if (IsKeyDown(KEY_I) || IsKeyDown(KEY_O) || mouseWheelMoved != 0.0f || Vector2Length(pinchMovement) != 0.0f)
		{
			shaderFractal.SetZoom(fractalParameters.zoom);
			UpdateGridIncrement();
		}
	}

	void TakeFractalScreenshot()
	{
		Image fractalImage = shaderFractal.GenImage(false, flipYAxis);

		try
		{
#ifdef PLATFORM_WEB
			//Export fractalImage to MEMFS
			bool exportSuccess = ExportImage(fractalImage, "fractal_screenshot.png");

			if (!exportSuccess)
				throw std::runtime_error("Failed to export fractal screenshot");
			
			//Save fractalImage from MEMFS to Disk
			emscripten_run_script("saveFileFromMEMFSToDisk(\"fractal_screenshot.png\", \"fractal_screenshot.png\")");

			notificationCurrent = Notification{ "Exported fractal screenshot!", 5.0f, WHITE };
#else //DESKTOP
			//Fractal_Screenshots directory
			std::filesystem::path fractalScreenshotsPath = std::filesystem::absolute(GetWorkingDirectory()).append("Fractal_Screenshots").make_preferred();

			//Create fractal screenshots directory if it doesn't exist
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

			//Get first unused screenshot number for path
			int num = 1;

			while (std::filesystem::exists(std::filesystem::absolute(GetWorkingDirectory()).append("Fractal_Screenshots/fractal_screenshot-" + std::to_string(num) + ".png").make_preferred()))
				num++;

			std::filesystem::path screenshotPath = std::filesystem::absolute(GetWorkingDirectory()).append("Fractal_Screenshots/fractal_screenshot-" + std::to_string(num) + ".png").make_preferred();

			//Export image to file
			bool exportSuccess = ExportImage(fractalImage, screenshotPath.string().c_str());

			if (!exportSuccess)
				throw std::runtime_error("Failed to export fractal screenshot to " + screenshotPath.string());

			notificationCurrent = Notification{ "Exported fractal screenshot to " + screenshotPath.string(), 5.0f, WHITE };
#endif
		}
		catch(std::exception& ex)
		{
			std::cout << ex.what() << std::endl;
			notificationCurrent = Notification{ ex };
		}

		UnloadImage(fractalImage);

		screenshotDeltaTime = -GetFrameTime();

		//TODO: play screenshot sfx here
	}
	#pragma endregion

	#pragma region UI function implementations

	Vector2 GetSelectedMouseFractalPosition(float snapWithinPixels)
	{
		Vector2 mouseScreenPosition = GetMousePosition();
		Vector2 mouseFractalPosition = GetScreenToFractalPosition(mouseScreenPosition, fractalParameters.position, fractalParameters.normalizedCenterOffset, fractalParameters.zoom, false, flipYAxis);
		
		Vector2 mouseSnapFractalPosition = SnapTo2DGrid(mouseFractalPosition, gridIncrement);
		Vector2 mouseSnapScreenPosition = GetFractalToScreenPosition(mouseSnapFractalPosition, fractalParameters.position, fractalParameters.normalizedCenterOffset, fractalParameters.zoom, false, flipYAxis);
	
		Vector2 screenDifference = Vector2Subtract(mouseScreenPosition, mouseSnapScreenPosition);

		//Snap to x grid line if close
		if (abs(screenDifference.x) < snapWithinPixels)
		{
			mouseFractalPosition.x = mouseSnapFractalPosition.x;
		}

		//Snap to y grid line if close
		if (abs(screenDifference.y) < snapWithinPixels)
		{
			mouseFractalPosition.y = mouseSnapFractalPosition.y;
		}

		return mouseFractalPosition;
	}

	bool CanHoldButton()
	{
		//Can keep holding or start holding button if click started on UI, and is not dragging a dot
		return activePressStartedOnUI && !isDraggingDot;
	}

	void DrawFractalGrid()
	{
		int screenWidth = GetScreenWidth();
		int screenHeight = GetScreenHeight();

		float screenScaleSqrt = sqrt(GetScreenScale(DESIGN_WIDTH, DESIGN_HEIGHT));

		float gridLineThickness = 2.0f * screenScaleSqrt;
		const float GRID_LINE_ALPHA = 0.25f;

		Font mainFontSemibold = Resources::GetFont("mainFontSemibold");

		Vector2 fractalCenterScreenPosition = GetFractalToScreenPosition(Vector2Zero(), fractalParameters.position, fractalParameters.normalizedCenterOffset, fractalParameters.zoom, false, flipYAxis);

		//Vertical grid line is on screen
		if (fractalCenterScreenPosition.x >= 0.0f || fractalCenterScreenPosition.x <= (float)screenWidth)
		{
			DrawLineEx(Vector2{ fractalCenterScreenPosition.x, 0.0f }, Vector2{ fractalCenterScreenPosition.x, (float)screenHeight }, gridLineThickness, WHITE);
		}

		//Horizontal grid line is on screen
		if (fractalCenterScreenPosition.y >= 0.0f || fractalCenterScreenPosition.y <= (float)screenHeight)
		{
			DrawLineEx(Vector2{ 0.0f, fractalCenterScreenPosition.y }, Vector2{ (float)screenWidth, fractalCenterScreenPosition.y }, gridLineThickness, WHITE);
		}

		// Axis number markers

		Vector2 minFractalPosition = GetScreenToFractalPosition(Vector2{ 0.0f, flipYAxis ? 0.0f : (float)screenHeight }, fractalParameters.position, fractalParameters.normalizedCenterOffset, fractalParameters.zoom, false, flipYAxis);
		Vector2 maxFractalPosition = GetScreenToFractalPosition(Vector2{ (float)screenWidth, flipYAxis ? (float)screenHeight : 0.0f }, fractalParameters.position, fractalParameters.normalizedCenterOffset, fractalParameters.zoom, false, flipYAxis);

		float markerLength = 20.0f * screenScaleSqrt;
		float numberFontSize = 20.0f * screenScaleSqrt;
		float numberPadding = 5.0f * screenScaleSqrt;

		//0 at center
		DrawTextEx(mainFontSemibold, "0", Vector2{fractalCenterScreenPosition.x + numberPadding, fractalCenterScreenPosition.y + numberPadding}, numberFontSize, numberFontSize / 10.0f ,WHITE);

		//Start at multiple of increment closest to min fractal x and draw markers until max fractal x

		//don't let the x-axis marker increment numbers go offscreen
		float incrementXPosY = std::clamp(
			fractalCenterScreenPosition.y + markerLength / 2.0f + numberPadding,
			numberPadding,
			std::max((float)screenHeight - numberFontSize - numberPadding, numberPadding)
		);

		int numIncrementsX = (int)((GetClosestLargerMultipleOf(maxFractalPosition.x, gridIncrement) - GetClosestSmallerMultipleOf(minFractalPosition.x, gridIncrement)) / gridIncrement) + 2;

		for (int incrementX = 0; incrementX < numIncrementsX; incrementX++)
		{
			float x = GetClosestSmallerMultipleOf(minFractalPosition.x, gridIncrement) + gridIncrement * (incrementX - 1);

			//don't draw 0, due to floating point imprecision we can't check if x is equal to 0.0f, increment divided by 2.0f so the first increment after 0.0 is drawn
			if (abs(x) < gridIncrement / 2.0f)
				continue;

			Vector2 fractalScreenPosition = GetFractalToScreenPosition(Vector2{ x, 0.0f }, fractalParameters.position, fractalParameters.normalizedCenterOffset, fractalParameters.zoom, false, flipYAxis);

			DrawLineEx(Vector2{ fractalScreenPosition.x, 0.0f }, Vector2{ fractalScreenPosition.x, (float)screenHeight }, gridLineThickness, ColorAlpha(WHITE, GRID_LINE_ALPHA));
			DrawLineEx(Vector2{fractalScreenPosition.x, fractalCenterScreenPosition.y - markerLength / 2.0f}, Vector2{ fractalScreenPosition.x, fractalCenterScreenPosition.y + markerLength / 2.0f }, gridLineThickness, WHITE);
		
			float numberLength = MeasureTextEx(mainFontSemibold, TextFormat("%g", x), numberFontSize, numberFontSize * FONT_SPACING_MULTIPLIER).x;

			DrawTextEx(
				mainFontSemibold,
				TextFormat("%g", x),
				Vector2{ fractalScreenPosition.x - numberLength / 2.0f, incrementXPosY },
				numberFontSize, 
				numberFontSize * FONT_SPACING_MULTIPLIER, 
				WHITE
			);
		}

		//Start at multiple of increment closest to min fractal y and draw markers until max fractal y

		//don't let the y-axis marker increment numbers go offscreen
		float incrementYPosX = std::clamp(
			fractalCenterScreenPosition.x - markerLength / 2.0f - numberPadding,
			numberPadding,
			std::max((float)screenWidth - numberPadding, numberPadding)
		);

		int numIncrementsY = (int)((GetClosestLargerMultipleOf(maxFractalPosition.y, gridIncrement) - GetClosestSmallerMultipleOf(minFractalPosition.y, gridIncrement)) / gridIncrement) + 2;

		for (int incrementY = 0; incrementY < numIncrementsY; incrementY++)
		{
			float y = GetClosestSmallerMultipleOf(minFractalPosition.y, gridIncrement) + gridIncrement * (incrementY - 1);

			//don't draw 0, due to floating point imprecision we can't check if y is equal to 0.0f, increment divided by 2.0f so the first increment after 0.0 is drawn
			if (abs(y) < gridIncrement / 2.0f)
				continue;

			Vector2 fractalScreenPosition = GetFractalToScreenPosition(Vector2{ 0.0f, y }, fractalParameters.position, fractalParameters.normalizedCenterOffset, fractalParameters.zoom, false, flipYAxis);
		
			DrawLineEx(Vector2{ 0.0f, fractalScreenPosition.y }, Vector2{ (float)screenWidth, fractalScreenPosition.y }, gridLineThickness, ColorAlpha(WHITE, GRID_LINE_ALPHA));
			DrawLineEx(Vector2{ fractalCenterScreenPosition.x - markerLength / 2.0f, fractalScreenPosition.y }, Vector2{ fractalCenterScreenPosition.x + markerLength / 2.0f, fractalScreenPosition.y }, gridLineThickness, WHITE);
	
			float numberLength = MeasureTextEx(mainFontSemibold, TextFormat("%gi", y), numberFontSize, numberFontSize * FONT_SPACING_MULTIPLIER).x;

			DrawTextEx(
				mainFontSemibold,
				TextFormat("%gi", y),
				Vector2{ std::max(incrementYPosX - numberLength, numberPadding), fractalScreenPosition.y - numberFontSize / 2.0f },
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

	void UpdateGridIncrement()
	{
		int screenWidth = GetScreenWidth();
		int screenHeight = GetScreenHeight();

		Vector2 minFractalPosition = GetScreenToFractalPosition(Vector2{ 0.0f, flipYAxis ? 0.0f : (float)screenHeight }, fractalParameters.position, fractalParameters.normalizedCenterOffset, fractalParameters.zoom, false, flipYAxis);
		Vector2 maxFractalPosition = GetScreenToFractalPosition(Vector2{ (float)screenWidth, flipYAxis ? (float)screenHeight : 0.0f }, fractalParameters.position, fractalParameters.normalizedCenterOffset, fractalParameters.zoom, false, flipYAxis);

		float minMaxDifference = std::max(maxFractalPosition.x - minFractalPosition.x, maxFractalPosition.y - minFractalPosition.y);

		int zoomLevel = 0;
		int defaultIncrement = 5;

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

		if (!warningEnded)
		{
			UpdateDrawWarning();
			return;
		}

		if (showGrid)
		{
			DrawFractalGrid();

			UpdateDrawDraggableDots();

			if (isDraggingDot)
			{
				cursorOnUI = true;
				activePressStartedOnUI = true;
			}
		}

		UpdateDrawFractalSelectionPanel();
		
		if (showInfoPanel)
			DrawInfoPanel();

		DrawFractalEquation();

		//Screenshot flash
		screenshotDeltaTime += deltaTime;

		if (screenshotDeltaTime <= 0.5f)
			DrawRectangle(0, 0, screenWidth, screenHeight, ColorAlpha(WHITE, 1.0f - (screenshotDeltaTime / 0.5f) * (screenshotDeltaTime / 0.5f)));
		
		//notification messages
		UpdateDrawNotification();

		if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
		{
			activePressStartedOnUI = false;
		}

		if (warningEndDeltaTime <= 1.0f)
			UpdateDrawWarning();
	}

	void UpdateDrawFractalSelectionPanel()
	{
		int screenWidth = GetScreenWidth();
		int screenHeight = GetScreenHeight();

		Font mainFontSemibold = Resources::GetFont("mainFontSemibold");

		Color mainBackgroundColor = ColorAlpha(DARKGRAY, 0.4f);
		Color altBackgroundColor = ColorAlpha(GRAY, 0.4f);

		float screenScale = GetScreenScale(DESIGN_WIDTH, DESIGN_HEIGHT);

		//Main rect

		float fractalSelectionHeight = 32.0f * screenScale;
		Rectangle fractalSelectionRect = Rectangle{ (float)screenWidth * 0.25f, (float)screenHeight - fractalSelectionHeight * 1.5f, (float)screenWidth * 0.5f, fractalSelectionHeight };

		DrawRectangleRec(Rectangle{ fractalSelectionRect.x + fractalSelectionRect.height, fractalSelectionRect.y, fractalSelectionRect.width - fractalSelectionRect.height * 2.0f, fractalSelectionRect.height }, mainBackgroundColor);

		CursorPanelCheck(fractalSelectionRect);

		//Fractal name

		const char* fractalName = GetFractalName(fractalParameters.type);
		float fractalNameFontSize = std::min(GetFontSizeForWidth(mainFontSemibold, fractalName, (fractalSelectionRect.width - fractalSelectionRect.height * 2.0f) * 0.8f, FONT_SPACING_MULTIPLIER), fractalSelectionHeight * 0.8f);
		Vector2 fractalNameTextSize = MeasureTextEx(mainFontSemibold, fractalName, fractalNameFontSize, fractalNameFontSize * FONT_SPACING_MULTIPLIER);
		DrawTextEx(mainFontSemibold, fractalName, Vector2{ fractalSelectionRect.x + fractalSelectionRect.width / 2.0f - fractalNameTextSize.x / 2.0f, fractalSelectionRect.y + fractalSelectionRect.height * 0.5f - fractalNameTextSize.y * 0.5f }, fractalNameFontSize, fractalNameFontSize * FONT_SPACING_MULTIPLIER, WHITE);

		//Previous fractal button

		Rectangle prevButtonRect = Rectangle{ fractalSelectionRect.x, fractalSelectionRect.y, fractalSelectionRect.height, fractalSelectionRect.height };
		Color prevButtonColor = DARKGRAY;

		if (IsRectangleHovered(prevButtonRect) && IsMouseButtonDown(MOUSE_BUTTON_LEFT))
			prevButtonColor = RAYWHITE;
		else if (IsRectangleHovered(prevButtonRect))
			prevButtonColor = LIGHTGRAY;

		DrawRectangleRec(prevButtonRect, ColorAlpha(prevButtonColor, 0.6f));

		float prevButtonFontSize = std::min(GetFontSizeForWidth(mainFontSemibold, "<", prevButtonRect.width * 0.8f, FONT_SPACING_MULTIPLIER), prevButtonRect.height * 0.8f);
		Vector2 prevButtonTextSize = MeasureTextEx(mainFontSemibold, "<", prevButtonFontSize, prevButtonFontSize * FONT_SPACING_MULTIPLIER);
		DrawTextEx(mainFontSemibold, "<", Vector2{ prevButtonRect.x + (prevButtonRect.width - prevButtonTextSize.x) * 0.5f, prevButtonRect.y + (prevButtonRect.height - prevButtonTextSize.y) * 0.5f }, prevButtonFontSize, prevButtonFontSize * FONT_SPACING_MULTIPLIER, WHITE);

		if (IsRectanglePressed(prevButtonRect))
			ChangeFractal((FractalType)((int)Wrap((float)fractalParameters.type - 1.0f, 0.0f, (float)NUM_FRACTAL_TYPES)));

		//Next fractal button

		Rectangle nextButtonRect = Rectangle{ fractalSelectionRect.x + fractalSelectionRect.width - fractalSelectionRect.height, fractalSelectionRect.y, fractalSelectionRect.height, fractalSelectionRect.height };
		Color nextButtonColor = DARKGRAY;

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

		DrawRectangleRec(buttonPanelRect, altBackgroundColor);

		CursorPanelCheck(buttonPanelRect);

		int buttonIndex = 0;

		Color defaultImageColor = WHITE;
		Color hoveredImageColor = LIGHTGRAY;
		Color pressedImageColor = DARKGRAY;

		//Info panel visibility
		Rectangle infoButtonRect = Rectangle{ buttonPanelRect.x + buttonPanelRect.height * buttonIndex, buttonPanelRect.y, buttonPanelRect.height, buttonPanelRect.height };

		DrawTextureButton(showInfoPanel ? Resources::GetTexture("icon_info_on") : Resources::GetTexture("icon_info_off"), infoButtonRect, defaultImageColor, hoveredImageColor, pressedImageColor);

		buttonIndex++;

		if (IsRectanglePressed(infoButtonRect))
			showInfoPanel = !showInfoPanel;

		//Screenshot button

		Rectangle screenshotButtonRect = Rectangle{ buttonPanelRect.x + buttonPanelRect.height * buttonIndex, buttonPanelRect.y, buttonPanelRect.height, buttonPanelRect.height };
	
		DrawTextureButton(Resources::GetTexture("icon_screenshot"), screenshotButtonRect, defaultImageColor, hoveredImageColor, pressedImageColor);

		buttonIndex++;

		if (IsRectanglePressed(screenshotButtonRect))
			TakeFractalScreenshot();

		//Grid visibility button

		Rectangle gridVisButtonRect = Rectangle{ buttonPanelRect.x + buttonPanelRect.height * buttonIndex, buttonPanelRect.y, buttonPanelRect.height, buttonPanelRect.height };

		DrawTextureButton(showGrid ? Resources::GetTexture("icon_grid_on") : Resources::GetTexture("icon_grid_off"), gridVisButtonRect, defaultImageColor, hoveredImageColor, pressedImageColor);

		buttonIndex++;

		if (IsRectanglePressed(gridVisButtonRect))
			showGrid = !showGrid;

		//Color banding button (if supported)
		if (FractalSupportsColorBanding(fractalParameters.type))
		{
			Rectangle colorBandingButtonRect = Rectangle{ buttonPanelRect.x + buttonPanelRect.height * buttonIndex, buttonPanelRect.y, buttonPanelRect.height, buttonPanelRect.height };
		
			DrawTextureButton(fractalParameters.colorBanding ? Resources::GetTexture("icon_colorbanding_on") : Resources::GetTexture("icon_colorbanding_off"), colorBandingButtonRect, defaultImageColor, hoveredImageColor, pressedImageColor);

			buttonIndex++;

			if (IsRectanglePressed(colorBandingButtonRect))
			{
				fractalParameters.colorBanding = !fractalParameters.colorBanding;
				shaderFractal.SetColorBanding(fractalParameters.colorBanding);
			}
		}

		//Power buttons (if supported)
		if (FractalSupportsPower(fractalParameters.type))
		{
			//Subtract
			Rectangle powerSubtractButtonRect = Rectangle{ buttonPanelRect.x + buttonPanelRect.height * buttonIndex, buttonPanelRect.y, buttonPanelRect.height, buttonPanelRect.height };

			DrawTextureButton(Resources::GetTexture("icon_power_subtract"), powerSubtractButtonRect, defaultImageColor, hoveredImageColor, pressedImageColor);

			buttonIndex++;

			if (IsRectangleDown(powerSubtractButtonRect) && CanHoldButton())
			{
				fractalParameters.power -= GetFrameTime();

				if (fractalParameters.power < 0.0f)
					fractalParameters.power = 0.0f;

				shaderFractal.SetPower(fractalParameters.power);
			}

			//Add
			Rectangle powerAddButtonRect = Rectangle{ buttonPanelRect.x + buttonPanelRect.height * buttonIndex, buttonPanelRect.y, buttonPanelRect.height, buttonPanelRect.height };

			DrawTextureButton(Resources::GetTexture("icon_power_add"), powerAddButtonRect, defaultImageColor, hoveredImageColor, pressedImageColor);

			buttonIndex++;

			if (IsRectangleDown(powerAddButtonRect) && CanHoldButton())
			{
				fractalParameters.power += GetFrameTime();
				shaderFractal.SetPower(fractalParameters.power);
			}

			//Floor
			Rectangle powerFloorButtonRect = Rectangle{ buttonPanelRect.x + buttonPanelRect.height * buttonIndex, buttonPanelRect.y, buttonPanelRect.height, buttonPanelRect.height };

			DrawTextureButton(Resources::GetTexture("icon_power_floor"), powerFloorButtonRect, defaultImageColor, hoveredImageColor, pressedImageColor);

			buttonIndex++;

			if (IsRectanglePressed(powerFloorButtonRect))
			{
				fractalParameters.power = floor(fractalParameters.power);
				shaderFractal.SetPower(fractalParameters.power);
			}
		}
	}

	void DrawFractalEquation()
	{
		int screenWidth = GetScreenWidth();
		int screenHeight = GetScreenHeight();

		Font mainFontSemibold = Resources::GetFont("mainFontSemibold");
		Color mainBackgroundColor = DARKGRAY;

		float screenScale = GetScreenScale(DESIGN_WIDTH, DESIGN_HEIGHT);
		float screenScaleSqrt = sqrt(screenScale);

		float positionY = 0.0f;

		float textPaddingX = 15.0f * screenScaleSqrt;
		float textPaddingY = 2.5f * screenScaleSqrt;

		//Polynomial

		int numRoots = GetFractalNumRoots(fractalParameters.type);

		if (numRoots > 0)
		{
			std::string polynomialText;
			
			switch (numRoots)
			{
				case 2:
					polynomialText = ComplexPolynomial::GetDegreeTwoFromRoots(ComplexFloat(fractalParameters.roots[0]), ComplexFloat(fractalParameters.roots[1]));
					break;
				case 3:
					polynomialText = ComplexPolynomial::GetDegreeThreeFromRoots(ComplexFloat(fractalParameters.roots[0]), ComplexFloat(fractalParameters.roots[1]), ComplexFloat(fractalParameters.roots[2]));
					break;
				case 4:
					polynomialText = ComplexPolynomial::GetDegreeFourFromRoots(ComplexFloat(fractalParameters.roots[0]), ComplexFloat(fractalParameters.roots[1]), ComplexFloat(fractalParameters.roots[2]), ComplexFloat(fractalParameters.roots[3]));
					break;
				case 5:
					polynomialText = ComplexPolynomial::GetDegreeFiveFromRoots(ComplexFloat(fractalParameters.roots[0]), ComplexFloat(fractalParameters.roots[1]), ComplexFloat(fractalParameters.roots[2]), ComplexFloat(fractalParameters.roots[3]), ComplexFloat(fractalParameters.roots[4]));
					break;
				default:
					polynomialText = "Can't get polynomial!";
					break;
			}

			float polynomialFontSize = std::min(32.0f * screenScaleSqrt, GetFontSizeForWidth(mainFontSemibold, polynomialText.c_str(), (float)screenWidth * 0.6f, FONT_SPACING_MULTIPLIER));

			Vector2 polynomialTextSize = MeasureTextEx(mainFontSemibold, polynomialText.c_str(), polynomialFontSize, polynomialFontSize * FONT_SPACING_MULTIPLIER);
			Rectangle polynomialRect = Rectangle{ (float)screenWidth * 0.5f - (polynomialTextSize.x + textPaddingX * 2.0f) / 2.0f, positionY, polynomialTextSize.x + 2.0f * textPaddingX, polynomialTextSize.y + 2.0f * textPaddingY };

			DrawRectangleRec(polynomialRect, ColorAlpha(GRAY, 0.4f));

			CursorPanelCheck(polynomialRect);

			DrawTextEx(mainFontSemibold, polynomialText.c_str(), Vector2{ polynomialRect.x + textPaddingX, polynomialRect.y + textPaddingY }, polynomialFontSize, polynomialFontSize * FONT_SPACING_MULTIPLIER, WHITE);
			positionY += polynomialRect.height;
		}

		//Equation

		float equationFontSize = std::min(32.0f * screenScaleSqrt, GetFontSizeForWidth(mainFontSemibold, GetFractalEquation(fractalParameters.type), (float)screenWidth * 0.25f, FONT_SPACING_MULTIPLIER));
		Vector2 equationTextSize = MeasureTextEx(mainFontSemibold, GetFractalEquation(fractalParameters.type), equationFontSize, equationFontSize * FONT_SPACING_MULTIPLIER);
		Rectangle equationRect = Rectangle{ (float)screenWidth * 0.5f - (equationTextSize.x + textPaddingX * 2.0f) / 2.0f, positionY, equationTextSize.x + 2.0f * textPaddingX, equationTextSize.y + 2.0f * textPaddingY };

		DrawRectangleRec(equationRect, ColorAlpha(DARKGRAY, 0.4f));

		CursorPanelCheck(equationRect);

		DrawTextEx(mainFontSemibold, GetFractalEquation(fractalParameters.type), Vector2{equationRect.x + textPaddingX, equationRect.y + textPaddingY}, equationFontSize, equationFontSize * FONT_SPACING_MULTIPLIER, WHITE);
	}

	void DrawInfoPanel()
	{
		int screenWidth = GetScreenWidth();
		int screenHeight = GetScreenHeight();

		float screenScale = GetScreenScale(DESIGN_WIDTH, DESIGN_HEIGHT);

		float statFontSize = 24.0f * sqrt(screenScale);

		Font mainFontSemibold = Resources::GetFont("mainFontSemibold");

		//info stats

		Vector2 textPadding = Vector2{ 5.0f * screenScale, 0.0f };
		Vector2 statPosition = Vector2{ 0.0f, 0.0f };

		Color unevenColor = ColorAlpha(GRAY, 0.4f);
		Color evenColor = ColorAlpha(DARKGRAY, 0.4f);

		int statIndex = 0;

		int fps = GetFPS();
		DrawStatInfo(TextFormat("FPS: %i", fps), statPosition, textPadding, statFontSize, fps <= 20 ? RED : WHITE, statIndex % 2 == 0 ? evenColor : unevenColor);
		statPosition.y += statFontSize + 2.0f * textPadding.y;
		statIndex++;

		DrawStatInfo(TextFormat("Position: x%g, y%g", fractalParameters.position.x, fractalParameters.position.y), statPosition, textPadding, statFontSize, WHITE, statIndex % 2 == 0 ? evenColor : unevenColor);
		statPosition.y += statFontSize + 2.0f * textPadding.y;
		statIndex++;
		
		DrawStatInfo(TextFormat("Zoom: %g", fractalParameters.zoom), statPosition, textPadding, statFontSize, WHITE, statIndex % 2 == 0 ? evenColor : unevenColor);
		statPosition.y += statFontSize + 2.0f * textPadding.y;
		statIndex++;

		DrawStatInfo(TextFormat("Max iterations: %i", fractalParameters.maxIterations), statPosition, textPadding, statFontSize, WHITE, statIndex % 2 == 0 ? evenColor : unevenColor);
		statPosition.y += statFontSize + 2.0f * textPadding.y;
		statIndex++;

		if (FractalSupportsPower(fractalParameters.type))
		{
			DrawStatInfo(TextFormat("n = %g", fractalParameters.power), statPosition, textPadding, statFontSize, WHITE, statIndex % 2 == 0 ? evenColor : unevenColor);
			statPosition.y += statFontSize + 2.0f * textPadding.y;
			statIndex++;
		}

		if (FractalSupportsC(fractalParameters.type))
		{
			DrawStatInfo(TextFormat("c = %g%+gi", fractalParameters.c.x, fractalParameters.c.y), statPosition, textPadding, statFontSize, WHITE, statIndex % 2 == 0 ? evenColor : unevenColor);
			statPosition.y += statFontSize + 2.0f * textPadding.y;
			statIndex++;
		}

		if (FractalSupportsA(fractalParameters.type))
		{
			DrawStatInfo(TextFormat("a = %g%+gi", fractalParameters.a.x, fractalParameters.a.y), statPosition, textPadding, statFontSize, WHITE, statIndex % 2 == 0 ? evenColor : unevenColor);
			statPosition.y += statFontSize + 2.0f * textPadding.y;
			statIndex++;
		}
	}

	void DrawStatInfo(const char* text, Vector2 position, Vector2 textPadding, float fontSize, Color textColor, Color backgroundColor)
	{
		Font mainFontSemibold = Resources::GetFont("mainFontSemibold");

		Vector2 infoSize = MeasureTextEx(mainFontSemibold, text, fontSize, fontSize * FONT_SPACING_MULTIPLIER);
		Rectangle infoRect = Rectangle{ position.x, position.y, infoSize.x + textPadding.x * 2.0f, fontSize + textPadding.y * 2.0f };

		DrawRectangleRec(infoRect, backgroundColor);
		CursorPanelCheck(infoRect);

		DrawTextEx(mainFontSemibold, text, Vector2Add(position, textPadding), fontSize, fontSize * FONT_SPACING_MULTIPLIER, textColor);
	}

	void DrawDraggableDot(Vector2 position, float radius, Color fillColor, Color outlineColor, bool isHovered, bool isDown)
	{
		DrawCircleV(position, radius, outlineColor);
		DrawCircleV(position, radius * 2.0f / 3.0f, isHovered ? ColorBrightness(fillColor, isDown ? -0.6f : -0.3f) : fillColor);
	}

	void UpdateDrawDraggableDots()
	{
		float screenScaleSqrt = sqrt(GetScreenScale(DESIGN_WIDTH, DESIGN_HEIGHT));

		float draggableDotRadius = 6.0f * screenScaleSqrt;

		float snapPixels = 8.0f * screenScaleSqrt;

		float labelFontSize = 32.0f * screenScaleSqrt;
		float valueFontSize = 24.0f * screenScaleSqrt;
		Vector2 labelOffset = Vector2{ 4.0f * screenScaleSqrt, -4.0f * screenScaleSqrt };

		Font& mainFontSemibold = Resources::GetFont("mainFontSemibold");

		if (FractalSupportsC(fractalParameters.type))
		{
			Vector2 cScreenPosition = GetFractalToScreenPosition(fractalParameters.c, fractalParameters.position, fractalParameters.normalizedCenterOffset, fractalParameters.zoom, false, flipYAxis);

			bool dotPressed = IsCirclePressed(cScreenPosition, draggableDotRadius);

			//Start drag
			if (dotPressed && !isDraggingDot)
			{
				isDraggingDot = true;
				draggingDotId = 0;
			}

			//Update drag
			if (draggingDotId == 0 && isDraggingDot)
			{
				fractalParameters.c = GetSelectedMouseFractalPosition(snapPixels);
				cScreenPosition = GetFractalToScreenPosition(fractalParameters.c, fractalParameters.position, fractalParameters.normalizedCenterOffset, fractalParameters.zoom, false, flipYAxis);
				shaderFractal.SetC(fractalParameters.c);
			}

			DrawDraggableDot(cScreenPosition, draggableDotRadius, WHITE, BLACK, IsCircleHovered(cScreenPosition, draggableDotRadius), draggingDotId == 0);
			DrawTextEx(mainFontSemibold, "c", Vector2Add(Vector2{ cScreenPosition.x, cScreenPosition.y - labelFontSize }, labelOffset), labelFontSize, labelFontSize * FONT_SPACING_MULTIPLIER, WHITE);
		
			Vector2 valueLabelSize = MeasureTextEx(mainFontSemibold, TextFormat("%g%+gi", fractalParameters.c.x, fractalParameters.c.y), valueFontSize, valueFontSize * FONT_SPACING_MULTIPLIER);
			DrawTextEx(mainFontSemibold, TextFormat("%g%+gi", fractalParameters.c.x, fractalParameters.c.y), Vector2Add(Vector2{ cScreenPosition.x - valueLabelSize.x / 2.0f, cScreenPosition.y - labelFontSize - valueLabelSize.y }, labelOffset), valueFontSize, valueFontSize * FONT_SPACING_MULTIPLIER, WHITE);
		}

		if (FractalSupportsA(fractalParameters.type))
		{
			Vector2 aScreenPosition = GetFractalToScreenPosition(fractalParameters.a, fractalParameters.position, fractalParameters.normalizedCenterOffset, fractalParameters.zoom, false, flipYAxis);

			bool dotPressed = IsCirclePressed(aScreenPosition, draggableDotRadius);

			//Start drag
			if (dotPressed && !isDraggingDot)
			{
				isDraggingDot = true;
				draggingDotId = 1;
			}

			//Update drag
			if (draggingDotId == 1 && isDraggingDot)
			{
				fractalParameters.a = GetSelectedMouseFractalPosition(snapPixels);
				aScreenPosition = GetFractalToScreenPosition(fractalParameters.a, fractalParameters.position, fractalParameters.normalizedCenterOffset, fractalParameters.zoom, false, flipYAxis);
				shaderFractal.SetA(fractalParameters.a);
			}

			DrawDraggableDot(aScreenPosition, draggableDotRadius, PINK, BLACK, IsCircleHovered(aScreenPosition, draggableDotRadius), draggingDotId == 1);
			DrawTextEx(mainFontSemibold, "a", Vector2Add(Vector2{ aScreenPosition.x, aScreenPosition.y - labelFontSize }, labelOffset), labelFontSize, labelFontSize * FONT_SPACING_MULTIPLIER, WHITE);
		
			Vector2 valueLabelSize = MeasureTextEx(mainFontSemibold, TextFormat("%g%+gi", fractalParameters.a.x, fractalParameters.a.y), valueFontSize, valueFontSize * FONT_SPACING_MULTIPLIER);
			DrawTextEx(mainFontSemibold, TextFormat("%g%+gi", fractalParameters.a.x, fractalParameters.a.y), Vector2Add(Vector2{ aScreenPosition.x - valueLabelSize.x / 2.0f, aScreenPosition.y - labelFontSize - valueLabelSize.y }, labelOffset), valueFontSize, valueFontSize * FONT_SPACING_MULTIPLIER, WHITE);
		}

		int numRoots = GetFractalNumRoots(fractalParameters.type);

		for (int i = 0; i < numRoots; i++)
		{
			int id = 1 + i + 1;

			Vector2 rootScreenPosition = GetFractalToScreenPosition(fractalParameters.roots[i], fractalParameters.position, fractalParameters.normalizedCenterOffset, fractalParameters.zoom, false, flipYAxis);

			bool dotPressed = IsCirclePressed(rootScreenPosition, draggableDotRadius);

			//Start drag
			if (dotPressed && !isDraggingDot)
			{
				isDraggingDot = true;
				draggingDotId = id;
			}

			//Update drag
			if (draggingDotId == id && isDraggingDot)
			{
				fractalParameters.roots[i] = GetSelectedMouseFractalPosition(snapPixels);
				GetFractalToScreenPosition(fractalParameters.roots[i], fractalParameters.position, fractalParameters.normalizedCenterOffset, fractalParameters.zoom, false, flipYAxis);
				shaderFractal.SetRoots(fractalParameters.roots.data(), numRoots);
			}

			DrawDraggableDot(rootScreenPosition, draggableDotRadius, ColorFromHSV(i * (360.0f / numRoots), 1.0f, 0.8f), BLACK, IsCircleHovered(rootScreenPosition, draggableDotRadius), draggingDotId == id);
			DrawTextEx(mainFontSemibold, TextFormat("r%i", i + 1), Vector2Add(Vector2{ rootScreenPosition.x, rootScreenPosition.y - labelFontSize }, labelOffset), labelFontSize, labelFontSize * FONT_SPACING_MULTIPLIER, WHITE);
			
			Vector2 valueLabelSize = MeasureTextEx(mainFontSemibold, TextFormat("%g%+gi", fractalParameters.roots[i].x, fractalParameters.roots[i].y), valueFontSize, valueFontSize * FONT_SPACING_MULTIPLIER);
			DrawTextEx(mainFontSemibold, TextFormat("%g%+gi", fractalParameters.roots[i].x, fractalParameters.roots[i].y), Vector2Add(Vector2{ rootScreenPosition.x - valueLabelSize.x / 2.0f, rootScreenPosition.y - labelFontSize - valueLabelSize.y }, labelOffset), valueFontSize, valueFontSize * FONT_SPACING_MULTIPLIER, WHITE);
		}

		//End drag
		if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
		{
			isDraggingDot = false;
			draggingDotId = -1;
			activePressStartedOnUI = false;
		}
	}

	void UpdateDrawNotification()
	{
		float deltaTime = GetFrameTime();

		notificationCurrent.deltaTime += deltaTime;

		if (notificationCurrent.deltaTime < notificationCurrent.timeSeconds)
		{
			int screenWidth = GetScreenWidth();
			int screenHeight = GetScreenHeight();

			float screenScale = GetScreenScale(DESIGN_WIDTH, DESIGN_HEIGHT);
			float screenScaleSqrt = sqrt(screenScale);

			Font mainFontSemibold = Resources::GetFont("mainFontSemibold");
			Color mainBackgroundColor = DARKGRAY;
			
			float alpha = 1.0f - std::max(notificationCurrent.deltaTime - (notificationCurrent.timeSeconds - 1.0f), 0.0f);

			float screenPadding = 10.0f * screenScale;
			float textPadding = 5.0f * screenScale;

			const char* notificationMessage = notificationCurrent.message.c_str();

			float notificationFontSize = std::min(30.0f * screenScaleSqrt, GetFontSizeForWidth(mainFontSemibold, notificationMessage, (float)screenWidth - 30.0f * screenScale, FONT_SPACING_MULTIPLIER));
			Vector2 notificationMessageSize = MeasureTextEx(mainFontSemibold, notificationCurrent.message.c_str(), notificationFontSize, notificationFontSize * FONT_SPACING_MULTIPLIER);
			Rectangle notificationRect = Rectangle{ screenPadding, screenHeight - notificationMessageSize.y - screenPadding - 2.0f * textPadding, notificationMessageSize.x + 2.0f * textPadding, notificationMessageSize.y + 2.0f * textPadding };

			CursorPanelCheck(notificationRect);

			DrawRectangleRec(notificationRect, ColorAlpha(mainBackgroundColor, 0.6f * alpha));
			DrawTextEx(mainFontSemibold, notificationMessage, Vector2{ screenPadding + textPadding, screenHeight - notificationMessageSize.y - screenPadding - textPadding }, notificationFontSize, notificationFontSize * FONT_SPACING_MULTIPLIER, ColorAlpha(notificationCurrent.color, alpha));
		}
	}

	void UpdateDrawWarning()
	{
		if (warningEnded)
			warningEndDeltaTime += GetFrameTime();

		if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
			warningEnded = true;

		Font mainFontSemibold = Resources::GetFont("mainFontSemibold");

		float alpha = 1.0f - warningEndDeltaTime / 1.0f;

		int screenWidth = GetScreenWidth();
		int screenHeight = GetScreenHeight();
		float screenScaleSqrt = sqrt(GetScreenScale(DESIGN_WIDTH, DESIGN_HEIGHT));

		DrawRectangle(0, 0, screenWidth, screenHeight, ColorAlpha(BLACK, alpha));

		const char* warningText = "!!! WARNING !!!";
		float warningFontSize = std::min(96.0f * screenScaleSqrt, GetFontSizeForWidth(mainFontSemibold, warningText, (float)screenWidth - 20.0f * screenScaleSqrt, FONT_SPACING_MULTIPLIER));
		Vector2 warningSize = MeasureTextEx(mainFontSemibold, warningText, warningFontSize, warningFontSize * FONT_SPACING_MULTIPLIER);
		DrawTextEx(mainFontSemibold, warningText, Vector2{ ((float)screenWidth - warningSize.x) / 2.0f, (float)screenHeight / 2.0f - warningSize.y }, warningFontSize, warningFontSize * FONT_SPACING_MULTIPLIER, ColorAlpha(RED, alpha));

		const char* messageText = "This tool contains MANY FLASHING LIGHTS & COLOURS.";
		float messageFontSize = std::min(48.0f * screenScaleSqrt, GetFontSizeForWidth(mainFontSemibold, messageText, (float)screenWidth - 20.0f * screenScaleSqrt, FONT_SPACING_MULTIPLIER));
		Vector2 messageSize = MeasureTextEx(mainFontSemibold, messageText, messageFontSize, messageFontSize * FONT_SPACING_MULTIPLIER);
		DrawTextEx(mainFontSemibold, messageText, Vector2{ ((float)screenWidth - messageSize.x) / 2.0f, (float)screenHeight / 2.0f }, messageFontSize, messageFontSize * FONT_SPACING_MULTIPLIER, ColorAlpha(WHITE, alpha));

		const char* proceedText = "Please click to proceed.";
		float proceedFontSize = std::min(48.0f * screenScaleSqrt, GetFontSizeForWidth(mainFontSemibold, proceedText, (float)screenWidth - 20.0f * screenScaleSqrt, FONT_SPACING_MULTIPLIER));
		Vector2 proceedSize = MeasureTextEx(mainFontSemibold, proceedText, proceedFontSize, proceedFontSize * FONT_SPACING_MULTIPLIER);
		DrawTextEx(mainFontSemibold, proceedText, Vector2{ ((float)screenWidth - proceedSize.x) / 2.0f, (float)screenHeight / 2.0f + messageSize.y }, proceedFontSize, proceedFontSize * FONT_SPACING_MULTIPLIER, ColorAlpha(WHITE, alpha));
	}

	void CursorPanelCheck(Rectangle rect)
	{
		if (IsRectangleHovered(rect))
			cursorOnUI = true;

		if (IsRectanglePressed(rect))
			activePressStartedOnUI = true;
	}
	#pragma endregion
}