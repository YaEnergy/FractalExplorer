#pragma once

#include <array>

#include "raylib.h"

namespace Explorer
{
	//Not including FRACTAL_UNKNOWN: not a fractal
	const int NUM_FRACTAL_TYPES = 10;

	const int NUM_MAX_ROOTS = 5;

	enum FractalType
	{
		FRACTAL_UNKNOWN = -1,
		FRACTAL_MULTIBROT = 0,
		FRACTAL_MULTICORN = 1,
		FRACTAL_BURNING_SHIP = 2,
		FRACTAL_JULIA = 3,
		FRACTAL_NEWTON_3DEG = 4,
		FRACTAL_NEWTON_4DEG = 5,
		FRACTAL_NEWTON_5DEG = 6,
		FRACTAL_NEWTON_SIN = 7,
		FRACTAL_POLYNOMIAL_2DEG = 8,
		FRACTAL_POLYNOMIAL_3DEG = 9,
	};

	const char* GetFractalName(FractalType);
	const char* GetFractalEquation(FractalType);

	//Parameters

	struct FractalParameters
	{
		FractalType type;

		Vector2 normalizedCenterOffset;

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
			type = FRACTAL_MULTIBROT;
			normalizedCenterOffset = Vector2{ -0.5f, -0.5f };
			position = Vector2{ 0.0f, 0.0f };
			zoom = 0.0f;
			maxIterations = 0;

			power = 2.0f;

			c = Vector2{ 0.0f, 0.0f };

			roots = std::array<Vector2, NUM_MAX_ROOTS>();
			roots.fill(Vector2{ 0.0f, 0.0f });

			a = Vector2{ 1.0f, 0.0f };

			colorBanding = false;
		}

		FractalParameters(FractalType type, Vector2 position, Vector2 normalizedCenterOffset, float zoom, int maxIterations, float power, Vector2 c, std::array<Vector2, NUM_MAX_ROOTS> roots, Vector2 a, bool colorBanding)
		{
			this->type = type;
			this->normalizedCenterOffset = normalizedCenterOffset;
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

	int GetFractalNumRoots(FractalType);

	bool FractalSupportsPower(FractalType);
	bool FractalSupportsC(FractalType);
	bool FractalSupportsA(FractalType);
	bool FractalSupportsColorBanding(FractalType);

	//Render Texture

	void InitFractalRenderTexture(int width, int height);
	void SetFractalRenderTextureSize(int width, int height);
	int GetFractalRenderTextureWidth();
	int GetFractalRenderTextureHeight();
	void UnloadFractalRenderTexture();

	//Shaders

	//Kinda wished not all fractal types had access to all the functions, but it's okay.

	class ShaderFractal
	{
		private:
			Shader fractalShader;
			FractalType type;
		public:
			FractalType GetFractalType() const;

			void SetNormalizedCenterOffset(Vector2);
			void SetWidthStretch(float);

			void SetPosition(Vector2);
			void SetZoom(float);
			void SetMaxIterations(int);

			void SetPower(float);
			void SetC(Vector2);

			void SetRoots(const Vector2* roots, int num);

			void SetA(Vector2);

			void SetColorBanding(bool);

			void Draw(Rectangle destination, bool flipX, bool flipY) const;

			Image GenImage(bool flipX, bool flipY) const;

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

	//Conversions

	float GetWidthStretchForSize(float width, float height);

	Vector2 GetFractalToRectPosition(Vector2 fractalPosition, Vector2 fractalOffset, Vector2 normalizedCenterOffset, Rectangle dest, float zoom, bool flipX = false, bool flipY = false);
	Vector2 GetRectToFractalPosition(Vector2 screenPosition, Vector2 fractalOffset, Vector2 normalizedCenterOffset, Rectangle src, float zoom, bool flipX = false, bool flipY = false);

	Vector2 GetScreenToFractalPosition(Vector2 screenPosition, Vector2 fractalOffset, Vector2 normalizedCenterOffset, float zoom, bool flipX = false, bool flipY = false);
	Vector2 GetFractalToScreenPosition(Vector2 fractalPosition, Vector2 fractalOffset, Vector2 normalizedCenterOffset, float zoom, bool flipX = false, bool flipY = false);
}