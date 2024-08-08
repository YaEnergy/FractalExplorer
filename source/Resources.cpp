#include "Resources.h"

#include <unordered_map>

std::unordered_map<std::string, Texture > textures;

std::unordered_map<std::string, Font> fonts;

void LoadResources()
{
	//Load fonts
	fonts.emplace("mainFontSemibold", LoadFontEx("assets/fonts/open-sans/OpenSans-Semibold.ttf", 64, NULL, 0)); //NULL & 0 for loading default character set
	fonts.emplace("mainFontRegular", LoadFontEx("assets/fonts/open-sans/OpenSans-Regular.ttf", 64, NULL, 0)); //NULL & 0 for loading default character set
}

void UnloadResources()
{
	//Unload textures
	std::unordered_map<std::string, Texture>::iterator textureIt = textures.begin();
	while (textureIt != textures.end())
	{
		UnloadTexture(textureIt->second);

		//go to next item
		textureIt++;
	}

	//Unload fonts
	std::unordered_map<std::string, Font>::iterator fontIt = fonts.begin();
	while (fontIt != fonts.end())
	{
		UnloadFont(fontIt->second);

		//go to next item
		fontIt++;
	}
}

Texture& GetTexture(std::string key)
{
	return textures.at(key);
}

Font& GetFont(std::string key)
{
	return fonts.at(key);
}