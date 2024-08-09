#pragma once

#include <string>

#include "raylib.h"

void LoadResources();
void UnloadResources();

Texture& GetTexture(std::string key);

Font& GetFont(std::string key);

Sound& GetSound(std::string key);