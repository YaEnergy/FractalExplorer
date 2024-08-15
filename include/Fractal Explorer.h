// Fractal Explorer.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <iostream>

// TODO: Reference additional headers your program requires here.

#if defined(PLATFORM_DESKTOP)
#define GLSL_VERSION            330
#else   // PLATFORM_ANDROID, PLATFORM_WEB
#define GLSL_VERSION            100
#endif

#ifdef WIN32RELEASE
int main();
#endif // RELEASE

const int DESIGN_WIDTH = 800;
const int DESIGN_HEIGHT = 480;

namespace Explorer
{
	void Init();

	void Deinit();

	void UpdateDrawFrame();
}
