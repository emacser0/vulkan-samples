#pragma once

#include "Texture.h"

#include <cstdint>
#include <array>
#include <vector>

class UTextureCube : public UTexture
{
public:
	DECLARE_OBJECT_BODY(UTextureCube, UTexture);

	UTextureCube();
	virtual ~UTextureCube();

	uint32_t GetWidth() const { return Width; }
	uint32_t GetHeight() const { return Height; }
	uint32_t GetNumChannels() const { return NumChannels; }
	const std::array<uint8_t*, 6> GetImages() const { return Images; }

	bool Load(const std::array<std::string, 6>& InFilenames);
	void Unload();

	virtual void CreateRenderTexture() override;

private:
	uint32_t Width;
	uint32_t Height;
	uint32_t NumChannels;

	std::array<uint8_t*, 6> Images;
};
