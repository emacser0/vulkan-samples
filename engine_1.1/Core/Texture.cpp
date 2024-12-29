#include "Texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <cassert>

FTexture::FTexture()
	: FAsset()
	, Width(0)
	, Height(0)
	, NumChannels(0)
	, Pixels(nullptr)
{

}

FTexture::~FTexture()
{
	if (Pixels != nullptr)
	{
		stbi_image_free(Pixels);
	}
}

bool FTexture::Load(const std::string& InFilename)
{
	int OutWidth, OutHeight, OutNumChannels;

	stbi_set_flip_vertically_on_load(true);

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
