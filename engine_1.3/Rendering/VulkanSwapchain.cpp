#include "VulkanSwapchain.h"
#include "VulkanContext.h"

FVulkanSwapchain::FVulkanSwapchain(FVulkanContext* InContext)
	: FVulkanObject(InContext)
	, Handle(VK_NULL_HANDLE)
	, Format(VK_FORMAT_UNDEFINED)
	, Extent({})
	, CurrentFrame(0)
	, CurrentImageIndex(0)
{
}

VkResult FVulkanSwapchain::Present(VkQueue InGfxQueue, VkQueue InPresentQueue, VkSemaphore InRenderFinishedSemaphore)
{
	return VK_SUCCESS;
}
