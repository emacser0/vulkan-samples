#pragma once

#include "vulkan/vulkan.h"
#include "Texture.h"

struct FVulkanTexture
{
public:
	uint32_t GetWidth() const { return Source.GetWidth(); }
	uint32_t GetHeight() const { return Source.GetHeight(); }
	uint32_t GetNumChannels() const { return Source.GetNumChannels(); }

	FTexture Source;
	VkImage Image;
	VkDeviceMemory Memory;
	VkImageView View;
};
