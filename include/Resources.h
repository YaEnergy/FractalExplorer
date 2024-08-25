#pragma once

#include <string>

#include "raylib.h"

namespace Explorer::Resources
{
	void Load();
	void Unload();

	Texture& GetTexture(std::string key);

	Font& GetFont(std::string key);

	Sound& GetSound(std::string key);
}
