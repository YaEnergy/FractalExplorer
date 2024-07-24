#pragma once

#include "raylib.h"

//Not including FRACTAL_UNKNOWN: not a fractal
const int NUM_FRACTAL_TYPES = 6;

enum FractalType
{
	FRACTAL_UNKNOWN = -1,
	FRACTAL_MANDELBROT = 0,
	FRACTAL_TRICORN = 1,
	FRACTAL_BURNING_SHIP = 2,
	FRACTAL_JULIA = 3,
	FRACTAL_MULTIBROT = 4,
	FRACTAL_MULTICORN = 5
};

struct FractalParameters
{
	FractalType type;

	Vector2 position;
	float zoom;
	int maxIterations;

	float power;

	Vector2 c;

	bool colorBanding;

	FractalParameters()
	{
		type = FRACTAL_MANDELBROT;
		position = Vector2{ 0.0f, 0.0f };
		zoom = 0.0f;
		maxIterations = 0;

		power = 2.0f;

		c = Vector2{ 0.0f, 0.0f };

		colorBanding = false;
	}

	FractalParameters(FractalType type, Vector2 position, float zoom, int maxIterations, float power, Vector2 c, bool colorBanding)
	{
		this->type = type;
		this->position = position;
		this->zoom = zoom;
		this->maxIterations = maxIterations;

		this->power = power;

		this->c = c;

		this->colorBanding = colorBanding;
	}
};

const char* GetFractalName(FractalType);

//Render Texture

void InitFractalRenderTexture(int width, int height);
void SetFractalRenderTextureSize(int width, int height);
int GetFractalRenderTextureWidth();
int GetFractalRenderTextureHeight();
void UnloadFractalRenderTexture();

//Shaders

class ShaderFractal
{
	private:
		Shader fractalShader;
		FractalType type;
	public:
		FractalType GetFractalType() const;

		void SetNormalizedScreenOffset(Vector2);
		void SetWidthStretch(float);

		void SetPosition(Vector2);
		void SetZoom(float);
		void SetMaxIterations(int);

		void SetPower(float);
		void SetC(Vector2);

		void SetColorBanding(bool);

		void Draw(Rectangle destination) const;

		void Unload();

		ShaderFractal()
		{
			fractalShader = { 0 };
			type = FRACTAL_UNKNOWN;
		}

		ShaderFractal(Shader shader, FractalType fractalType)
		{
			fractalShader = shader;
			type = fractalType;
		}
};

ShaderFractal LoadShaderFractal(FractalType);

//Drawing

void SaveShaderFractalToImage(const ShaderFractal& shaderFractal);



