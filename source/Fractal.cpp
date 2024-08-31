#include "Fractal.h"

#include "raylib.h"
#include "raymath.h"

namespace Explorer
{
	const char* fractalShaderDirectory =
#ifdef PLATFORM_WEB 
		"assets/shaders/v100";
#else //Desktop
		"assets/shaders/v330";
#endif

	const char* fragmentShaderFileNames[NUM_FRACTAL_TYPES] = {
		"multibrotFractal.frag",
		"multicornFractal.frag",
		"burningShipFractal.frag",
		"juliaFractal.frag",
		"newtonFractal_3.frag",
		"newtonFractal_4.frag",
		"newtonFractal_5.frag",
		"newtonFractal_sin.frag",
		"polynomialFractal_2.frag",
		"polynomialFractal_3.frag"
	};

	RenderTexture fractalRenderTexture;

	const char* GetFractalName(FractalType fractalType)
	{
		switch (fractalType)
		{
			case FRACTAL_MULTIBROT:
				return "Mandelbrot (+Multi) Set Fractal";
			case FRACTAL_MULTICORN:
				return "Tricorn (+Multi) Fractal";
			case FRACTAL_BURNING_SHIP:
				return "Burning Ship Fractal";
			case FRACTAL_JULIA:
				return "Julia Set Fractal";
			case FRACTAL_NEWTON_3DEG:
				return "Generalized Newton Fractal - 3rd-degree polynomial";
			case FRACTAL_NEWTON_4DEG:
				return "Generalized Newton Fractal - 4th-degree polynomial";
			case FRACTAL_NEWTON_5DEG:
				return "Generalized Newton Fractal - 5th-degree polynomial";
			case FRACTAL_POLYNOMIAL_2DEG:
				return "P(z)+c, deg(P)=2";
			case FRACTAL_POLYNOMIAL_3DEG:
				return "P(z)+c, deg(P)=3";
			case FRACTAL_NEWTON_SIN:
				return "Generalized Newton Fractal - P(z) = sin(z)";
			default: //Or FRACTAL_UNKNOWN
				return "UNKNOWN FRACTAL";
		}
	}

	const char* GetFractalEquation(FractalType fractalType)
	{
		switch (fractalType)
		{
			case FRACTAL_MULTIBROT:
				return "z ^ n + c";
			case FRACTAL_MULTICORN:
				return "(Re(z) - Im(z) i) ^ n + c";
			case FRACTAL_BURNING_SHIP:
				return "(|Re(z) + |Im (z)| i) ^ 2 + c";
			case FRACTAL_JULIA:
				return "z ^ n + c";
			case FRACTAL_NEWTON_3DEG:
				return "z - a * (P(z) / P'(z)), deg P = 3";
			case FRACTAL_NEWTON_4DEG:
				return "z - a * (P(z) / P'(z)), deg P = 4";
			case FRACTAL_NEWTON_5DEG:
				return "z - a * (P(z) / P'(z)), deg P = 5";
			case FRACTAL_POLYNOMIAL_2DEG:
				return "P(z) + c, deg P = 2";
			case FRACTAL_POLYNOMIAL_3DEG:
				return "P(z) + c, deg P = 2";
			case FRACTAL_NEWTON_SIN:
				return "z - a * (P(z) / P'(z)), P(z) = sin(z)";
			default: //Or FRACTAL_UNKNOWN
				return "UNKNOWN FRACTAL EQUATION";
		}
	}

	#pragma region Parameters

	int GetFractalNumRoots(FractalType type)
	{
		switch (type)
		{
			case FRACTAL_POLYNOMIAL_2DEG:
				return 2;
			case FRACTAL_POLYNOMIAL_3DEG:
				return 3;
			case FRACTAL_NEWTON_3DEG:
				return 3;
			case FRACTAL_NEWTON_4DEG:
				return 4;
			case FRACTAL_NEWTON_5DEG:
				return 5;
			default:
				return 0;
		}
	}

	bool FractalSupportsPower(FractalType type)
	{
		return type == FRACTAL_MULTIBROT || type == FRACTAL_MULTICORN || type == FRACTAL_JULIA;
	}

	bool FractalSupportsC(FractalType type)
	{
		return type == FRACTAL_JULIA;
	}

	bool FractalSupportsA(FractalType type)
	{
		return type == FRACTAL_NEWTON_3DEG || type == FRACTAL_NEWTON_4DEG || type == FRACTAL_NEWTON_5DEG || type == FRACTAL_NEWTON_SIN;
	}

	bool FractalSupportsColorBanding(FractalType type)
	{
		return type != FRACTAL_NEWTON_3DEG && type != FRACTAL_NEWTON_4DEG && type != FRACTAL_NEWTON_5DEG && type != FRACTAL_NEWTON_SIN;
	}

	#pragma endregion

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
		Shader fractalShader = LoadShader(NULL, TextFormat("%s/%s", fractalShaderDirectory, fragmentShaderFileNames[type]));

		return ShaderFractal(fractalShader, type);
	}

	FractalType ShaderFractal::GetFractalType() const
	{
		return type;
	}

	void ShaderFractal::SetNormalizedCenterOffset(Vector2 offset)
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

	void ShaderFractal::SetRoots(const Vector2* roots, int num)
	{
		SetShaderValueV(fractalShader, GetShaderLocation(fractalShader, "roots"), roots, SHADER_UNIFORM_VEC2, num);
	}

	void ShaderFractal::SetA(Vector2 a)
	{
		SetShaderValue(fractalShader, GetShaderLocation(fractalShader, "a"), &a, SHADER_UNIFORM_VEC2);
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
	void ShaderFractal::Draw(Rectangle destination, bool flipX, bool flipY) const
	{
		int renderWidth = GetRenderWidth();
		int renderHeight = GetRenderHeight();

		BeginShaderMode(fractalShader);
		{
			//Fractal is drawn flipped because of flipped render texture, so the vertically flipped version is actually the correct side up
			//if flipY is true it will be flipped again
			Rectangle fractalSource = { 0.0f, 0.0f, flipX ? -(float)fractalRenderTexture.texture.width : (float)fractalRenderTexture.texture.width, flipY ? (float)fractalRenderTexture.texture.height : -(float)fractalRenderTexture.texture.height };

			DrawTexturePro(fractalRenderTexture.texture, fractalSource, destination, { 0.0f, 0.0f }, 0.0f, WHITE);
		}
		EndShaderMode();
	}

	Image ShaderFractal::GenImage(bool flipX, bool flipY) const
	{
		//Fractal render
		RenderTexture2D fractalImageRender = LoadRenderTexture(fractalRenderTexture.texture.width, fractalRenderTexture.texture.height);

		BeginTextureMode(fractalImageRender);
		{
			ClearBackground(BLACK);

			//render textures are flipped, y is flipped again
			Draw(Rectangle{ 0.0f, 0.0f, (float)fractalImageRender.texture.width, (float)fractalImageRender.texture.height }, flipX, !flipY);
		}
		EndTextureMode();

		Image fractalImage = LoadImageFromTexture(fractalImageRender.texture);

		//Unload
		UnloadRenderTexture(fractalImageRender);

		return fractalImage;
	}
	#pragma endregion

	#pragma region Conversion
	float GetWidthStretchForSize(float width, float height)
	{
		return 1.0f / (width / height);
	}

	Vector2 GetFractalToRectPosition(Vector2 fractalPosition, Vector2 fractalOffset, Vector2 normalizedCenterOffset, Rectangle dest, float zoom, bool flipX, bool flipY)
	{
		//Fractal is drawn flipped because of flipped render texture, so the vertically flipped version is actually the correct side up
		//if flipY is true it will be flipped again

		float widthStretch = GetWidthStretchForSize(dest.width, dest.height);

		Vector2 screenPosition = Vector2{
			flipX ? dest.x + (-(fractalPosition.x - fractalOffset.x) * widthStretch * zoom - normalizedCenterOffset.x) * dest.width : dest.x + ((fractalPosition.x - fractalOffset.x) * widthStretch * zoom - normalizedCenterOffset.x) * dest.width,
			flipY ? dest.y + ((fractalPosition.y - fractalOffset.y) * zoom - normalizedCenterOffset.y) * dest.height : dest.y + (-(fractalPosition.y - fractalOffset.y) * zoom - normalizedCenterOffset.y) * dest.height
		};

		return screenPosition;
	}

	Vector2 GetRectToFractalPosition(Vector2 screenPosition, Vector2 fractalOffset, Vector2 normalizedCenterOffset, Rectangle src, float zoom, bool flipX, bool flipY)
	{
		//Fractal is drawn flipped because of flipped render texture, so the vertically flipped version is actually the correct side up
		//if flipY is true it will be flipped again

		Vector2 fragTexCoord = Vector2{ (screenPosition.x - src.x) / src.width, (screenPosition.y - src.y) / src.height };
		float widthStretch = GetWidthStretchForSize(src.width, src.height);

		Vector2 fractalPosition = Vector2{
			flipX ? (fragTexCoord.x + normalizedCenterOffset.x) / -(widthStretch * zoom) + fractalOffset.x : (fragTexCoord.x + normalizedCenterOffset.x) / (widthStretch * zoom) + fractalOffset.x,
			flipY ? (fragTexCoord.y + normalizedCenterOffset.y) / zoom + fractalOffset.y : (fragTexCoord.y + normalizedCenterOffset.y) / -zoom + fractalOffset.y
		};

		return Vector2{ fractalPosition.x, fractalPosition.y };
	}

	Vector2 GetScreenToFractalPosition(Vector2 screenPosition, Vector2 fractalOffset, Vector2 normalizedCenterOffset, float zoom, bool flipX, bool flipY)
	{
		int screenWidth = GetScreenWidth();
		int screenHeight = GetScreenHeight();

		return GetRectToFractalPosition(
			screenPosition, 
			fractalOffset, 
			normalizedCenterOffset, 
			Rectangle{ 0.0f, 0.0f, (float)screenWidth, (float)screenHeight }, 
			zoom, 
			flipX, 
			flipY
		);
	}

	Vector2 GetFractalToScreenPosition(Vector2 fractalPosition, Vector2 fractalOffset, Vector2 normalizedCenterOffset, float zoom, bool flipX, bool flipY)
	{
		int screenWidth = GetScreenWidth();
		int screenHeight = GetScreenHeight();

		return GetFractalToRectPosition(
			fractalPosition,
			fractalOffset,
			normalizedCenterOffset,
			Rectangle{ 0.0f, 0.0f, (float)screenWidth, (float)screenHeight },
			zoom,
			flipX,
			flipY
		);
	}

	#pragma endregion
}