#pragma once

#include "Asset.h"

#include <cstdint>
#include <string>

class FTextureSource : public FAsset
{
public:
	FTextureSource();
	virtual ~FTextureSource();

	uint32_t GetWidth() const { return Width; }
	uint32_t GetHeight() const { return Height; }
	uint32_t GetNumChannels() const { return NumChannels; }
	const uint8_t* GetPixels() const { return Pixels; }

	bool Load(const std::string& InFilename);

private:
	uint32_t Width;
	uint32_t Height;
	uint32_t NumChannels;

	uint8_t* Pixels;
};
