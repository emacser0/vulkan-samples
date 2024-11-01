#pragma once

#include "vulkan/vulkan.h"
#include "TextureSource.h"

struct FVulkanTexture
{
public:
	uint32_t GetWidth() const { return Source.GetWidth(); }
	uint32_t GetHeight() const { return Source.GetHeight(); }
	uint32_t GetNumChannels() const { return Source.GetNumChannels(); }

	FTextureSource Source;
	VkImage Image;
	VkDeviceMemory Memory;
	VkImageView View;
};
