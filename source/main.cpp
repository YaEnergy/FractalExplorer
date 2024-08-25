// main.cpp : Defines the entry point for the application.
//

#include "raylib.h"

#include "Fractal Explorer.h"

#include "Resources.h"

#ifdef WIN32RELEASE
int WinMain()
{
	return main();
}
#endif

void DrawLoadingFrame();

void DrawLoadingFrame()
{
	BeginDrawing();

	ClearBackground(BLACK);

	int screenWidth = GetScreenWidth();
	int screenHeight = GetScreenHeight();

	const int LOADING_FONT_SIZE = 36;

	const char* loadingNameText = "FRACTAL EXPLORER";
	DrawText(loadingNameText, (screenWidth - MeasureText(loadingNameText, LOADING_FONT_SIZE)) / 2, (screenHeight - LOADING_FONT_SIZE) / 2, LOADING_FONT_SIZE, WHITE);

	const char* loadingText = "LOADING...";
	DrawText(loadingText, (screenWidth - MeasureText(loadingText, LOADING_FONT_SIZE)) / 2, (screenHeight - LOADING_FONT_SIZE) / 2 + LOADING_FONT_SIZE, LOADING_FONT_SIZE, WHITE);

	EndDrawing();
}

int main()
{
	//Init
	InitWindow(DESIGN_WIDTH, DESIGN_HEIGHT, "Complex Fractal Explorer");
	InitAudioDevice();

	SetWindowMinSize(80, 48);
	SetWindowState(FLAG_WINDOW_RESIZABLE);
	SetExitKey(KEY_NULL);

	DrawLoadingFrame();

	//starting up the program from outside the application directory (ex: with a terminal, works with windows shortcuts) 
	//sets the working directory to outside the application directory which causes it to attempt loading resources from 
	//the incorrect directories, so set working directory to application directory
	ChangeDirectory(GetApplicationDirectory());

	Explorer::Resources::Load();

	//Set up icon
	Image windowIcon = LoadImage("assets/fractalExplorerIcon.png");
	ImageFormat(&windowIcon, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8); //Required for window icon
	SetWindowIcon(windowIcon);
	UnloadImage(windowIcon);

	Explorer::Init();

	//TODO: Emscripten modifications

	while (!WindowShouldClose())
	{
		Explorer::UpdateDrawFrame();
	}

	//Deinit
	Explorer::Resources::Unload();
	Explorer::Deinit();

	CloseAudioDevice();
	CloseWindow();

	return 0;
}