#pragma once

#include <cstdint>

struct FTexture
{
	int32_t Width;
	int32_t Height;
	int32_t Channels;

	uint8_t* Pixels;
};
