#include "Resources.h"

#include <unordered_map>

#include "raylib.h"

namespace Explorer::Resources
{
	std::unordered_map<std::string, Texture> textures;

	std::unordered_map<std::string, Font> fonts;

	std::unordered_map<std::string, Sound> sounds;

	void Load()
	{
		//Load textures
		textures.emplace("icon_screenshot", LoadTexture("assets/icons/icon_screenshot.png"));

		textures.emplace("icon_grid_on", LoadTexture("assets/icons/icon_grid_on.png"));
		textures.emplace("icon_grid_off", LoadTexture("assets/icons/icon_grid_off.png"));

		textures.emplace("icon_colorbanding_on", LoadTexture("assets/icons/icon_colorbanding_on.png"));
		textures.emplace("icon_colorbanding_off", LoadTexture("assets/icons/icon_colorbanding_off.png"));

		textures.emplace("icon_power_add", LoadTexture("assets/icons/icon_power_add.png"));
		textures.emplace("icon_power_subtract", LoadTexture("assets/icons/icon_power_subtract.png"));
		textures.emplace("icon_power_floor", LoadTexture("assets/icons/icon_power_floor.png"));

		textures.emplace("icon_info_on", LoadTexture("assets/icons/icon_info_on.png"));
		textures.emplace("icon_info_off", LoadTexture("assets/icons/icon_info_off.png"));

		//Load fonts
		fonts.emplace("mainFontSemibold", LoadFontEx("assets/fonts/open-sans/OpenSans-Semibold.ttf", 128, NULL, 0)); //NULL & 0 for loading default character set
		fonts.emplace("mainFontRegular", LoadFontEx("assets/fonts/open-sans/OpenSans-Regular.ttf", 128, NULL, 0)); //NULL & 0 for loading default character set
	}

	void Unload()
	{
		//Unload textures
		std::unordered_map<std::string, Texture>::iterator textureIt = textures.begin();
		while (textureIt != textures.end())
		{
			UnloadTexture(textureIt->second);

			//go to next item
			textureIt++;
		}

		textures.clear();

		//Unload fonts
		std::unordered_map<std::string, Font>::iterator fontIt = fonts.begin();
		while (fontIt != fonts.end())
		{
			UnloadFont(fontIt->second);

			//go to next item
			fontIt++;
		}

		fonts.clear();

		//Unload sounds
		std::unordered_map<std::string, Sound>::iterator soundIt = sounds.begin();
		while (soundIt != sounds.end())
		{
			UnloadSound(soundIt->second);

			//go to next item
			soundIt++;
		}

		sounds.clear();
	}

	Texture& GetTexture(std::string key)
	{
		return textures.at(key);
	}

	Font& GetFont(std::string key)
	{
		return fonts.at(key);
	}

	Sound& GetSound(std::string key)
	{
		return sounds.at(key);
	}
}
