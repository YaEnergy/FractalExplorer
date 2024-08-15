#pragma once

#include <string>

#include "raylib.h"

namespace Resources
{
	void Load();
	void Unload();

	Texture& GetTexture(std::string key);

	Font& GetFont(std::string key);

	Sound& GetSound(std::string key);
}
