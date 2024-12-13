#pragma once

#include "VulkanObject.h"

#include "vulkan/vulkan.h"

#include <vector>

class FVulkanSwapchain : public FVulkanObject
{
public:
	FVulkanSwapchain(class FVulkanContext* InContext);
	virtual ~FVulkanSwapchain() = default;

	static FVulkanSwapchain* Create(
		class FVulkanContext* InContext,
		const VkSwapchainCreateInfoKHR& InSwapchainCI);

	virtual void Destroy() override;

	VkSwapchainKHR GetHandle() const { return Swapchain; }
	VkFormat GetFormat() const { return Format; }
	VkExtent2D GetExtent() const { return Extent; }

	uint32_t GetImageCount() const { return ImageCount; }
	const std::vector<VkImage>& GetImages() const { return Images; }
	const std::vector<VkImageView> GetImageViews() const { return ImageViews; }

	uint32_t GetCurrentImageIndex() const { return CurrentImageIndex; }

	VkResult Present(VkQueue InGfxQueue, VkQueue InPresentQueue, VkSemaphore InRenderFinishedSemaphore);
	VkResult AcquireNextImage(VkSemaphore InImageAcquiredSemaphore);

private:
	VkSwapchainKHR Swapchain;
	VkFormat Format;
	VkExtent2D Extent;

	uint32_t ImageCount;
	std::vector<VkImage> Images;
	std::vector<VkImageView> ImageViews;

	uint32_t CurrentImageIndex;
};
