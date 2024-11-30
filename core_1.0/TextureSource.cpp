#include "TextureSource.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <cassert>

UTextureSource::UTextureSource()
	: Width(0)
	, Height(0)
	, NumChannels(0)
	, Pixels(nullptr)
{

}

UTextureSource::~UTextureSource()
{
	if (Pixels != nullptr)
	{
		stbi_image_free(Pixels);
	}
}

bool UTextureSource::Load(const std::string& InFilename)
{
	int TextureWidth, TextureHeight, TextureChannels;

	int OutWidth, OutHeight, OutNumChannels;

	Pixels = stbi_load(InFilename.c_str(), &OutWidth, &OutHeight, &OutNumChannels, STBI_rgb_alpha);
	if (Pixels == nullptr)
	{
		return false;
	}

	assert(OutWidth >= 0);
	assert(OutHeight >= 0);
	assert(OutNumChannels >= 0);

	Width = static_cast<uint32_t>(OutWidth);
	Height = static_cast<uint32_t>(OutHeight);
	NumChannels = static_cast<uint32_t>(OutNumChannels);

	return true;
}
