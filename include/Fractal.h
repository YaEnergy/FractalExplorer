#pragma once

#include <array>

#include "raylib.h"

//Not including FRACTAL_UNKNOWN: not a fractal
const int NUM_FRACTAL_TYPES = 7;

const int NUM_MAX_ROOTS = 3;

enum FractalType
{
	FRACTAL_UNKNOWN = -1,
	FRACTAL_MANDELBROT = 0,
	FRACTAL_TRICORN = 1,
	FRACTAL_BURNING_SHIP = 2,
	FRACTAL_JULIA = 3,
	FRACTAL_MULTIBROT = 4,
	FRACTAL_MULTICORN = 5,
	FRACTAL_NEWTON_3DEG = 6
};

struct FractalParameters
{
	FractalType type;

	Vector2 position;
	float zoom;
	int maxIterations;

	float power;

	Vector2 c;

	std::array<Vector2, NUM_MAX_ROOTS> roots;

	Vector2 a;

	bool colorBanding;

	FractalParameters()
	{
		type = FRACTAL_MANDELBROT;
		position = Vector2{ 0.0f, 0.0f };
		zoom = 0.0f;
		maxIterations = 0;

		power = 2.0f;

		c = Vector2{ 0.0f, 0.0f };

		//Default roots are the roots to most known Newton Fractal (P(z) = z^3 - 1)
		roots[0] = Vector2{ 1.0f, 0.0f };
		roots[1] = Vector2{ -0.5f, 0.866025f };
		roots[2] = Vector2{ -0.5f, -0.866025f };

		a = Vector2{ 1.0f, 0.0f };

		colorBanding = false;
	}

	FractalParameters(FractalType type, Vector2 position, float zoom, int maxIterations, float power, Vector2 c, std::array<Vector2, NUM_MAX_ROOTS> roots, Vector2 a, bool colorBanding)
	{
		this->type = type;
		this->position = position;
		this->zoom = zoom;
		this->maxIterations = maxIterations;

		this->power = power;

		this->c = c;

		this->roots = roots;

		this->a = a;

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

		void SetRoots(const Vector2* roots, int num);

		void SetA(Vector2);

		void SetColorBanding(bool);

		bool SupportsPower() const;
		bool SupportsC() const;

		int GetNumSettableRoots() const;

		bool SupportsA() const;

		bool SupportsColorBanding() const;

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



