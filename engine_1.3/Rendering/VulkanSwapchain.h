#pragma once

#include "VulkanObject.h"

#include "vulkan/vulkan.h"

#include <vector>

class FVulkanSwapchain : public FVulkanObject
{
public:
	FVulkanSwapchain(class FVulkanContext* InContext);
	virtual ~FVulkanSwapchain() = default;

	VkSwapchainKHR GetHandle() const { return Handle; }
	VkFormat GetFormat() const { return Format; }
	VkExtent2D GetExtent() const { return Extent; }

	VkResult Present(VkQueue InGfxQueue, VkQueue InPresentQueue, VkSemaphore InRenderFinishedSemaphore);

private:
	VkSwapchainKHR Handle;
	VkFormat Format;
	VkExtent2D Extent;

	uint32_t CurrentFrame;
	uint32_t CurrentImageIndex;

	std::vector<VkSemaphore> ImageAcquiredSemaphores;
};
