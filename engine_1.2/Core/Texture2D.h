#pragma once

#include "Texture.h"

class UTexture2D : public UTexture
{
public:
	DECLARE_OBJECT_BODY(UTexture2D, UTexture);

	UTexture2D();
	virtual ~UTexture2D();

	uint32_t GetWidth() const { return Width; }
	uint32_t GetHeight() const { return Height; }
	uint32_t GetNumChannels() const { return NumChannels; }
	const uint8_t* GetPixels() const { return Pixels; }

	bool Load(const std::string& InFilename, bool InbIsNormal = false);
	void Unload();

	virtual void CreateRenderTexture();

private:
	uint32_t Width;
	uint32_t Height;
	uint32_t NumChannels;
	bool bIsNormal;

	uint8_t* Pixels;
};
