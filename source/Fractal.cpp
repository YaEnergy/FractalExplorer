#include "Fractal.h"

#include <iostream>
#include <filesystem>

#include "raylib.h"
#include "raymath.h"

const char* fragmentShaderFilePaths[NUM_FRACTAL_TYPES] = {
	"assets/shaders/mandelbrotFractal.frag",
	"assets/shaders/tricornFractal.frag",
	"assets/shaders/burningShipFractal.frag",
	"assets/shaders/juliaFractal.frag",
	"assets/shaders/multibrotFractal.frag",
	"assets/shaders/multicornFractal.frag"
};

RenderTexture fractalRenderTexture;

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
		case FRACTAL_MULTICORN:
			return "Multicorn Set Fractal";
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
ShaderFractal LoadShaderFractal(FractalType type)
{
	Shader fractalShader = LoadShader(NULL, fragmentShaderFilePaths[type]);

	return ShaderFractal(fractalShader, type);
}

FractalType ShaderFractal::GetFractalType() const
{
	return type;
}

void ShaderFractal::SetNormalizedScreenOffset(Vector2 offset)
{
	SetShaderValue(fractalShader, GetShaderLocation(fractalShader, "offset"), &offset, SHADER_UNIFORM_VEC2);
}

void ShaderFractal::SetWidthStretch(float widthStretch)
{
	SetShaderValue(fractalShader, GetShaderLocation(fractalShader, "widthStretch"), &widthStretch, SHADER_UNIFORM_FLOAT);
}

void ShaderFractal::SetPosition(Vector2 position)
{
	SetShaderValue(fractalShader, GetShaderLocation(fractalShader, "position"), &position, SHADER_UNIFORM_VEC2);
}

void ShaderFractal::SetZoom(float zoom)
{
	SetShaderValue(fractalShader, GetShaderLocation(fractalShader, "zoom"), &zoom, SHADER_UNIFORM_FLOAT);
}

void ShaderFractal::SetMaxIterations(int maxIterations)
{
	SetShaderValue(fractalShader, GetShaderLocation(fractalShader, "maxIterations"), &maxIterations, SHADER_UNIFORM_INT);
}

void ShaderFractal::SetPower(float power)
{
	SetShaderValue(fractalShader, GetShaderLocation(fractalShader, "power"), &power, SHADER_UNIFORM_FLOAT);
}

void ShaderFractal::SetC(Vector2 c)
{
	SetShaderValue(fractalShader, GetShaderLocation(fractalShader, "c"), &c, SHADER_UNIFORM_VEC2);
}

void ShaderFractal::SetColorBanding(bool colorBanding)
{
	//SetShaderValue has no way of setting uniform bools, so an integer is used instead
	int colorBandingInt = colorBanding ? 1 : 0;
	SetShaderValue(fractalShader, GetShaderLocation(fractalShader, "colorBanding"), &colorBandingInt, SHADER_UNIFORM_INT);
}

void ShaderFractal::Unload()
{
	UnloadShader(fractalShader);
}
#pragma endregion

#pragma region Drawing
void ShaderFractal::Draw(Rectangle destination) const
{
	int renderWidth = GetRenderWidth();
	int renderHeight = GetRenderHeight();

	BeginShaderMode(fractalShader);
	{
		//Fractal is drawn flipped because of flipped render texture
		Rectangle fractalSource = { 0.0f, 0.0f, (float)fractalRenderTexture.texture.width, -(float)fractalRenderTexture.texture.height };

		DrawTexturePro(fractalRenderTexture.texture, fractalSource, destination, { 0.0f, 0.0f }, 0.0f, WHITE);
	}
	EndShaderMode();
}

void SaveShaderFractalToImage(const ShaderFractal& shaderFractal)
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

		shaderFractal.Draw(Rectangle{0.0f, 0.0f, (float)fractalImageRender.texture.width, (float)fractalImageRender.texture.height});
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