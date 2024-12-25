#include "TextureSource.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <cassert>

FTextureSource::FTextureSource()
	: Width(0)
	, Height(0)
	, NumChannels(0)
	, Pixels(nullptr)
{

}

FTextureSource::~FTextureSource()
{
	if (Pixels != nullptr)
	{
		stbi_image_free(Pixels);
	}
}

bool FTextureSource::Load(const std::string& InFilename)
{
	int TextureWidth, TextureHeight, TextureChannels;

	Pixels = stbi_load(InFilename.c_str(), &TextureWidth, &TextureHeight, &TextureChannels, STBI_rgb_alpha);
	if (Pixels == nullptr)
	{
		return false;
	}

	assert(TextureWidth >= 0);
	assert(TextureHeight >= 0);
	assert(TextureChannels >= 0);

	Width = static_cast<uint32_t>(TextureWidth);
	Height = static_cast<uint32_t>(TextureHeight);
	NumChannels = static_cast<uint32_t>(TextureChannels);

	return true;
}
