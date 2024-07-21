#include "Fractal.h"

#include <iostream>
#include <filesystem>

#include "raylib.h"
#include "raymath.h"

Shader fractalShaders[NUM_FRACTAL_TYPES] = { 0 };
RenderTexture fractalRenderTexture;

void UpdateFractalShaderValues(const FractalParameters& parameters);

const char* GetFractalName(FractalType fractalType)
{
	switch (fractalType)
	{
		case FRACTAL_MANDELBROT:
			return "Mandelbrot Set Fractal";
		case FRACTAL_TRICORN:
			return "Tricorn Fractal (Mandelbar)";
		case FRACTAL_BURNING_SHIP:
			return "Burning Ship Fractal";
		case FRACTAL_JULIA:
			return "Julia Set Fractal";
		case FRACTAL_MULTIBROT:
			return "Multibrot Set Fractal";
	}
}

#pragma region Render Texture
void InitFractalRenderTexture(int width, int height)
{
	fractalRenderTexture = LoadRenderTexture(width, height);

	//Fill fractal render texture
	BeginTextureMode(fractalRenderTexture);
	{
		ClearBackground(BLACK);
		DrawRectangle(0, 0, fractalRenderTexture.texture.width, fractalRenderTexture.texture.height, BLACK);
	}
	EndTextureMode();
}

void SetFractalRenderTextureSize(int width, int height)
{
	UnloadRenderTexture(fractalRenderTexture);

	InitFractalRenderTexture(width, height);
}

int GetFractalRenderTextureWidth()
{
	return fractalRenderTexture.texture.width;
}

int GetFractalRenderTextureHeight()
{
	return fractalRenderTexture.texture.height;
}

void UnloadFractalRenderTexture()
{
	UnloadRenderTexture(fractalRenderTexture);
}
#pragma endregion

#pragma region Shaders
void LoadFractalShaders()
{
	fractalShaders[FRACTAL_MANDELBROT] = LoadShader(NULL, "assets/shaders/mandelbrotFractal.frag");
	fractalShaders[FRACTAL_TRICORN] = LoadShader(NULL, "assets/shaders/tricornFractal.frag");
	fractalShaders[FRACTAL_BURNING_SHIP] = LoadShader(NULL, "assets/shaders/burningShipFractal.frag");
	fractalShaders[FRACTAL_JULIA] = LoadShader(NULL, "assets/shaders/juliaFractal.frag");
	fractalShaders[FRACTAL_MULTIBROT] = LoadShader(NULL, "assets/shaders/multibrotFractal.frag");
}

void UpdateFractalShaderValues(const FractalParameters& parameters)
{
	Shader fractalShader = fractalShaders[parameters.type];

	//display shader values
	Vector2 offset = { -0.5f, -0.5f };
	SetShaderValue(fractalShader, GetShaderLocation(fractalShader, "offset"), &offset, SHADER_UNIFORM_VEC2);

	float widthStretch = 1.0f / ((float)GetRenderWidth() / (float)GetRenderHeight());
	SetShaderValue(fractalShader, GetShaderLocation(fractalShader, "widthStretch"), &widthStretch, SHADER_UNIFORM_FLOAT);

	// Main complex fractal shader values
	SetShaderValue(fractalShader, GetShaderLocation(fractalShader, "position"), &parameters.position, SHADER_UNIFORM_VEC2);
	SetShaderValue(fractalShader, GetShaderLocation(fractalShader, "zoom"), &parameters.zoom, SHADER_UNIFORM_FLOAT);
	SetShaderValue(fractalShader, GetShaderLocation(fractalShader, "maxIterations"), &parameters.maxIterations, SHADER_UNIFORM_INT);

	//extra options
	//SetShaderValue has no way of setting uniform bools, so an integer is used instead
	int colorBandingInt = parameters.colorBanding ? 1 : 0;
	SetShaderValue(fractalShader, GetShaderLocation(fractalShader, "colorBanding"), &colorBandingInt, SHADER_UNIFORM_INT);

	//extra fractal shader values
	switch (parameters.type)
	{
		case FRACTAL_JULIA:
			SetShaderValue(fractalShader, GetShaderLocation(fractalShader, "c"), &parameters.c, SHADER_UNIFORM_VEC2);
			SetShaderValue(fractalShader, GetShaderLocation(fractalShader, "power"), &parameters.power, SHADER_UNIFORM_FLOAT);
			break;
		case FRACTAL_MULTIBROT:
			SetShaderValue(fractalShader, GetShaderLocation(fractalShader, "power"), &parameters.power, SHADER_UNIFORM_FLOAT);
			break;
		default:
			break;
	}
}

void UnloadFractalShaders()
{
	for (int i = 0; i < NUM_FRACTAL_TYPES; i++)
	{
		UnloadShader(fractalShaders[i]);
	}
}
#pragma endregion

#pragma region Drawing
void DrawFractal(const FractalParameters& parameters, Rectangle destination)
{
	int renderWidth = GetRenderWidth();
	int renderHeight = GetRenderHeight();

	Shader fractalShader = fractalShaders[parameters.type];

	UpdateFractalShaderValues(parameters);

	BeginShaderMode(fractalShader);
	{
		Rectangle fractalSource = { 0.0f, 0.0f, (float)fractalRenderTexture.texture.width, (float)fractalRenderTexture.texture.height };

		DrawTexturePro(fractalRenderTexture.texture, fractalSource, destination, { 0.0f, 0.0f }, 0.0f, WHITE);
	}
	EndShaderMode();
}

void SaveFractalToImage(const FractalParameters& parameters)
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

	//Fractal render
	RenderTexture2D fractalImageRender = LoadRenderTexture(fractalRenderTexture.texture.width, fractalRenderTexture.texture.height);

	BeginTextureMode(fractalImageRender);
	{
		ClearBackground(BLACK);

		DrawFractal(parameters, Rectangle{0.0f, 0.0f, (float)fractalImageRender.texture.width, (float)fractalImageRender.texture.height});
	}
	EndTextureMode();

	Image fractalImage = LoadImageFromTexture(fractalImageRender.texture);

	//render textures are flipped
	ImageFlipVertical(&fractalImage);

	//Save & Export
	int num = 1;

	while (std::filesystem::exists(TextFormat("%s\\fractal_screenshot-%i.png", fractalScreenshotsPath.string().c_str(), num)))
		num++;

	ExportImage(fractalImage, TextFormat("%s\\fractal_screenshot-%i.png", fractalScreenshotsPath.string().c_str(), num));

	//Unload
	UnloadRenderTexture(fractalImageRender);
	UnloadImage(fractalImage);
}
#pragma endregion