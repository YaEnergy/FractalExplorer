// main.cpp : Defines the entry point for the application.
//

#include "raylib.h"

#include "Fractal Explorer.h"

#include "Resources.h"

#ifdef PLATFORM_WEB
#include <emscripten/emscripten.h>
#endif

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

	const float LOADING_FONT_SIZE = 36.0f;
	const float LOADING_SPACING_MULTIPLIER = 1.0f / 15.0f;

	Font loadingFont = LoadFontEx("assets/fonts/open-sans/OpenSans-Semibold.ttf", 36, NULL, 0);

	const char* loadingNameText = "FRACTAL EXPLORER";
	Vector2 loadingNameSize = MeasureTextEx(loadingFont, loadingNameText, LOADING_FONT_SIZE, LOADING_FONT_SIZE * LOADING_SPACING_MULTIPLIER);
	DrawTextEx(loadingFont, loadingNameText, Vector2{ ((float)screenWidth - loadingNameSize.x) / 2.0f, ((float)screenHeight - loadingNameSize.y) / 2.0f }, LOADING_FONT_SIZE, LOADING_FONT_SIZE * LOADING_SPACING_MULTIPLIER, WHITE);
	
	const char* loadingText = "LOADING...";
	Vector2 loadingSize = MeasureTextEx(loadingFont, loadingText, LOADING_FONT_SIZE, LOADING_FONT_SIZE * LOADING_SPACING_MULTIPLIER);
	DrawTextEx(loadingFont, loadingText, Vector2{ ((float)screenWidth - loadingSize.x) / 2.0f, ((float)screenHeight - loadingSize.y) / 2.0f + loadingNameSize.y }, LOADING_FONT_SIZE, LOADING_FONT_SIZE * LOADING_SPACING_MULTIPLIER, WHITE);

	EndDrawing();

	UnloadFont(loadingFont);
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

#ifdef PLATFORM_WEB
	emscripten_set_main_loop(Explorer::UpdateDrawFrame, 0, 1);
#else
	while (!WindowShouldClose())
	{
		Explorer::UpdateDrawFrame();
	}
#endif

	//Deinit
	Explorer::Resources::Unload();
	Explorer::Deinit();

	CloseAudioDevice();
	CloseWindow();

	return 0;
}