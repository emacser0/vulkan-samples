#pragma once

#include "VulkanObject.h"

#include "vulkan/vulkan.h"

#include <cstdint>

class FVulkanImage : public FVulkanObject
{
public:
	FVulkanImage(class FVulkanContext* InContext);

	virtual void Destroy() override;

	void CreateImage(
		VkExtent3D InExtent,
		uint32_t InMipLevels,
		uint32_t InArrayLayers,
		VkFormat InFormat,
		VkImageType InImageType,
		VkImageTiling InTiling,
		VkImageUsageFlags InUsage,
		VkMemoryPropertyFlags InProperties,
		VkImageCreateFlags InFlags = 0);

	void CreateView(
		VkImageViewType InImageViewType,
		VkImageAspectFlags InAspectFlags);
	void DestroyView();

	void SetExtent(VkExtent3D InExtent) { Extent = InExtent; }
	void SetMipLevels(uint32_t InMipLevels) { MipLevels = InMipLevels; }
	void SetArrayLayers(uint32_t InArrayLayers) { ArrayLayers = InArrayLayers; }
	void SetFormat(VkFormat InFormat) { Format = InFormat; }
	void SetImageType(VkImageType InImageType) { ImageType = InImageType; }
	void SetTiling(VkImageTiling InTiling) { Tiling = InTiling; }
	void SetUsage(VkImageUsageFlags InUsage) { Usage = InUsage; }
	void SetProperties(VkMemoryPropertyFlags InProperties) { Properties = InProperties; }

	VkImage GetImage() const { return Image; }
	VkDeviceMemory GetMemory() const { return Memory; }
	VkImageView GetView() const { return View; }

private:
	VkExtent3D Extent;
	uint32_t MipLevels;
	uint32_t ArrayLayers;
	VkFormat Format;
	VkImageType ImageType;
	VkImageTiling Tiling;
	VkImageUsageFlags Usage;
	VkMemoryPropertyFlags Properties;

	VkImage Image;
	VkDeviceMemory Memory;
	VkImageView View;
};
