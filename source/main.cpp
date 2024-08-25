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

int main()
{
	//Init
	InitWindow(DESIGN_WIDTH, DESIGN_HEIGHT, "Complex Fractal Explorer");
	InitAudioDevice();

	SetWindowMinSize(80, 48);
	SetWindowState(FLAG_WINDOW_RESIZABLE);
	SetExitKey(KEY_NULL);

	//Linux: using ./ from outside the application directory sets the working directory to outside the application directory
	//which causes it to attempt loading resources from the incorrect directories, so set working directory to application directory
	ChangeDirectory(GetApplicationDirectory());

	Resources::Load();

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
	Resources::Unload();
	Explorer::Deinit();

	CloseAudioDevice();
	CloseWindow();

	return 0;
}