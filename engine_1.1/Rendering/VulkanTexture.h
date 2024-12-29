#pragma once

#include "VulkanObject.h"

#include "vulkan/vulkan.h"

class FVulkanTexture : public FVulkanObject
{
public:
	FVulkanTexture(FVulkanContext* InContext);
	virtual ~FVulkanTexture();

	void LoadSource(const class FTexture& InSource);

	uint32_t GetWidth() const { return Width; }
	uint32_t GetHeight() const { return Height; }
	VkFormat GetFormat() const { return Format; }

	VkImage GetImage() const { return Image; }
	VkDeviceMemory GetMemory() const { return Memory; }
	VkImageView GetView() const { return View; }

private:
	VkImage Image;
	VkDeviceMemory Memory;
	VkImageView View;

	uint32_t Width;
	uint32_t Height;
	VkFormat Format;

	bool bLoaded;
};
