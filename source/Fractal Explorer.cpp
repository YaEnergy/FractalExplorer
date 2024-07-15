// Fractal Explorer.cpp : Defines the entry point for the application.
//

#include "Fractal Explorer.h"

#include <cmath>
#include <string>
#include <vector>

#include "raylib.h"
#include "raymath.h"

#include "Fractals/Mandelbrot.h"
#include "ComplexNumbers/ComplexFloat.h"

const int NUM_FRACTAL_TYPES = 3;

enum FractalType
{
	FRACTAL_MANDELBROT = 0,
	FRACTAL_BURNING_SHIP = 1,
	FRACTAL_JULIA = 2
};

const char* fractalNames[NUM_FRACTAL_TYPES] =
{
	"Mandelbrot Set Fractal",
	"Burning Ship Fractal",
	"Julia Set Fractal"
};

Shader fractalShaders[NUM_FRACTAL_TYPES] = { 0 };

FractalType selectedFractalType = FRACTAL_MANDELBROT;
RenderTexture fractalRenderTexture;

//Main complex fractal parameters

Vector2 position = Vector2{ 0.0f, 0.0f };
float zoom = 1.0f;
int maxIterations = 200;

// Extra julia set fractal parameters

Vector2 juliaC = Vector2{ 0.0f, 0.0f };
float juliaPower = 2.0f;

// Extra options

bool colorBanding = false;
bool showDebugInfo = false;

//Delta times

float zoomDeltaTime = 0.0f;


void UpdateDrawFrame();

void Update();
void Draw();

#pragma region Fractal functions
void ResetFractalParameters();
void SetFractalType(FractalType fractalType);

void UpdateFractalShaderValues();

void UpdateFractal();
void UpdateFractalControls();
void UpdateFractalCamera();

void DrawFractal();
#pragma endregion

#pragma region UI functions
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
	const int START_WINDOW_WIDTH = 800;
	const int START_WINDOW_HEIGHT = 480;

	//Init
	InitWindow(START_WINDOW_WIDTH, START_WINDOW_HEIGHT, "Complex Fractal Explorer");
	InitAudioDevice();

	fractalRenderTexture = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());

	SetWindowState(FLAG_WINDOW_RESIZABLE);
	
	//Load fractal shaders
	fractalShaders[FRACTAL_MANDELBROT] = LoadShader(NULL, "assets/shaders/mandelbrotFractal.frag");
	fractalShaders[FRACTAL_BURNING_SHIP] = LoadShader(NULL, "assets/shaders/burningShipFractal.frag");
	fractalShaders[FRACTAL_JULIA] = LoadShader(NULL, "assets/shaders/juliaFractal.frag");

	SetFractalType(FRACTAL_MANDELBROT);

	//TODO: Emscripten modifications

	while (!WindowShouldClose())
	{
		UpdateDrawFrame();
	}

	//Deinit
	UnloadRenderTexture(fractalRenderTexture);

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

	if (IsKeyPressed(KEY_E))
		colorBanding = !colorBanding;

	if (IsKeyPressed(KEY_SPACE))
		showDebugInfo = !showDebugInfo;
}

void Draw()
{
	BeginDrawing();
	{
		ClearBackground(BLACK);

		DrawFractal();

		DrawUI();
	}
	EndDrawing();
}

#pragma region Fractal function implementations

void ResetFractalParameters()
{
	position = Vector2{ 0.0f, 0.0f };
	maxIterations = 300;
	zoom = 1.0f;

	//julia set fractal parameters
	if (selectedFractalType == FRACTAL_JULIA)
	{
		juliaC = Vector2{ 0.0f, 0.0f };
		juliaPower = 2.0f;
	}
}

void SetFractalType(FractalType fractalType)
{
	selectedFractalType = fractalType;

	ResetFractalParameters();
}

void UpdateFractalShaderValues()
{
	Shader fractalShader = fractalShaders[selectedFractalType];

	//display shader values
	Vector2 offset = { -0.5f, -0.5f };
	SetShaderValue(fractalShader, GetShaderLocation(fractalShader, "offset"), &offset, SHADER_UNIFORM_VEC2);

	float widthStretch = 1.0f / ((float)GetScreenWidth() / (float)GetScreenHeight());
	SetShaderValue(fractalShader, GetShaderLocation(fractalShader, "widthStretch"), &widthStretch, SHADER_UNIFORM_FLOAT);

	// Main complex fractal shader values
	SetShaderValue(fractalShader, GetShaderLocation(fractalShader, "position"), &position, SHADER_UNIFORM_VEC2);
	SetShaderValue(fractalShader, GetShaderLocation(fractalShader, "zoom"), &zoom, SHADER_UNIFORM_FLOAT);
	SetShaderValue(fractalShader, GetShaderLocation(fractalShader, "maxIterations"), &maxIterations, SHADER_UNIFORM_INT);

	//extra options
	//SetShaderValue has no way of setting uniform bools, so an integer is used instead
	int colorBandingInt = colorBanding ? 1 : 0;
	SetShaderValue(fractalShader, GetShaderLocation(fractalShader, "colorBanding"), &colorBandingInt, SHADER_UNIFORM_INT);

	//julia set fractal shader values
	if (selectedFractalType == FRACTAL_JULIA)
	{
		SetShaderValue(fractalShader, GetShaderLocation(fractalShader, "c"), &juliaC, SHADER_UNIFORM_VEC2);
		SetShaderValue(fractalShader, GetShaderLocation(fractalShader, "power"), &juliaPower, SHADER_UNIFORM_FLOAT);
	}
}

void UpdateFractal()
{
	int screenWidth = GetScreenWidth();
	int screenHeight = GetScreenHeight();

	if (IsKeyPressed(KEY_ONE))
		SetFractalType(FRACTAL_MANDELBROT);
	else if (IsKeyPressed(KEY_TWO))
		SetFractalType(FRACTAL_BURNING_SHIP);
	else if (IsKeyPressed(KEY_THREE))
		SetFractalType(FRACTAL_JULIA);

	if (IsKeyPressed(KEY_T))
		SetFractalType((FractalType)(((int)selectedFractalType + 1) % NUM_FRACTAL_TYPES));

	Shader fractalShader = fractalShaders[selectedFractalType];

	//Update fractal render texture if screen size has changed
	if (screenWidth != fractalRenderTexture.texture.width || screenHeight != fractalRenderTexture.texture.height)
	{
		UnloadRenderTexture(fractalRenderTexture);
		fractalRenderTexture = LoadRenderTexture(screenWidth, screenHeight);
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
		maxIterations++;
	else if (IsKeyPressed(KEY_KP_SUBTRACT))
		maxIterations--;

	if (selectedFractalType == FRACTAL_JULIA)
	{
		//Power changing using keys
		if (IsKeyDown(KEY_F))
			juliaPower -= 1.0f * deltaTime;
		else if (IsKeyDown(KEY_G))
			juliaPower += 1.0f * deltaTime;

		//Round power down
		if (IsKeyPressed(KEY_H))
			juliaPower = floor(juliaPower);

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

			juliaC.x += mouseDelta.x / (float)fractalRenderTexture.texture.width / zoom;
			juliaC.y -= mouseDelta.y / (float)fractalRenderTexture.texture.height / zoom;
		}
	}
}

void UpdateFractalCamera()
{
	float deltaTime = GetFrameTime();

	//Camera panning using keys
	float movementSpeed = IsKeyDown(KEY_LEFT_SHIFT) ? 0.5f : 0.1f;

	if (IsKeyDown(KEY_LEFT))
		position.x -= movementSpeed * deltaTime / zoom;
	else if (IsKeyDown(KEY_RIGHT))
		position.x += movementSpeed * deltaTime / zoom;

	if (IsKeyDown(KEY_UP))
		position.y -= movementSpeed * deltaTime / zoom;
	else if (IsKeyDown(KEY_DOWN))
		position.y += movementSpeed * deltaTime / zoom;

	//Camera panning using mouse
	if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
	{
		Vector2 mouseDelta = GetMouseDelta();

		position.x -= mouseDelta.x / (float)fractalRenderTexture.texture.width / zoom; /// qualityDivision / zoom;
		position.y -= mouseDelta.y / (float)fractalRenderTexture.texture.height / zoom; /// qualityDivision / zoom;
	}

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

	//Zooming using mouse wheel
	//up scroll: zoom in
	//down scroll: zoom out
	float mouseWheelMoved = GetMouseWheelMove();
	zoom += zoom * 0.1f * mouseWheelMoved;

	//Zooming using pinching
	Vector2 pinchMovement = GetGesturePinchVector();
	zoom += zoom * 0.5f * Vector2Length(pinchMovement);

	if (zoom <= 0.01f)
		zoom = 0.01f;
}

void DrawFractal()
{
	int screenWidth = GetScreenWidth();
	int screenHeight = GetScreenHeight();

	Shader fractalShader = fractalShaders[selectedFractalType];

	UpdateFractalShaderValues();

	BeginTextureMode(fractalRenderTexture);
	{
		ClearBackground(BLACK);
		DrawRectangle(0, 0, fractalRenderTexture.texture.width, fractalRenderTexture.texture.height, BLACK);
	}
	EndTextureMode();

	BeginShaderMode(fractalShader);
	{
		Rectangle fractalSource = { 0.0f, 0.0f, (float)fractalRenderTexture.texture.width, (float)fractalRenderTexture.texture.height };
		Rectangle fractalDestination = { 0.0f, 0.0f, (float)screenWidth, (float)screenHeight };

		DrawTexturePro(fractalRenderTexture.texture, fractalSource, fractalDestination, { 0.0f, 0.0f }, 0.0f, WHITE);
	}
	EndShaderMode();
}

#pragma endregion

#pragma region UI

void UpdateUI()
{

}

void DrawUI()
{
	int screenWidth = GetScreenWidth();
	int screenHeight = GetScreenHeight();

	if (showDebugInfo)
	{
		DrawFPS(10, 10);
		DrawText(TextFormat("Position : %f, %f", position.x, position.y), 10, 10 + 24, 24, GREEN);
		DrawText(TextFormat("Zoom: %f", zoom), 10, 10 + 24 * 2, 24, GREEN);
		DrawText(TextFormat("Max iterations: %i", maxIterations), 10, 10 + 24 * 3, 24, GREEN);
		DrawText(TextFormat("Screen size: %ix%i", screenWidth, screenHeight), 10, 10 + 24 * 4, 24, GREEN);
		DrawText(TextFormat("Color banding?: %s", (colorBanding ? "Yes" : "Reduced")), 10, 10 + 24 * 5, 24, GREEN);
		DrawText(TextFormat("Pixels: %i", screenWidth * screenHeight), 10, 10 + 24 * 6, 24, GREEN);

		DrawText(fractalNames[selectedFractalType], 10, 10 + 24 * 7, 24, WHITE);

		if (selectedFractalType == FRACTAL_JULIA)
		{
			DrawText(TextFormat("Julia C : %f, %f", juliaC.x, juliaC.y), 10, 10 + 24 * 8, 24, WHITE);
			DrawText(TextFormat("Julia Pow : %f", juliaPower), 10, 10 + 24 * 9, 24, WHITE);
		}
	}
}

#pragma endregion function implementations