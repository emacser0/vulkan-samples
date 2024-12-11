#include "VulkanSwapchain.h"
#include "VulkanContext.h"
#include "VulkanHelpers.h"

FVulkanSwapchain::FVulkanSwapchain(FVulkanContext* InContext)
	: FVulkanObject(InContext)
	, Swapchain(VK_NULL_HANDLE)
	, Format(VK_FORMAT_UNDEFINED)
	, Extent({})
	, ImageCount(0)
	, CurrentImageIndex(0)
{
}

FVulkanSwapchain* FVulkanSwapchain::Create(
	FVulkanContext* InContext,
	const VkSwapchainCreateInfoKHR& InSwapchainCI)
{
	FVulkanSwapchain* Swapchain = InContext->CreateObject<FVulkanSwapchain>();
	Swapchain->Format = InSwapchainCI.imageFormat;
	Swapchain->Extent = InSwapchainCI.imageExtent;

	VkDevice Device = InContext->GetDevice();
	VK_ASSERT(vkCreateSwapchainKHR(Device, &InSwapchainCI, nullptr, &Swapchain->Swapchain));

	vkGetSwapchainImagesKHR(Device, Swapchain->Swapchain, &Swapchain->ImageCount, nullptr);

	Swapchain->Images.resize(Swapchain->ImageCount);
	vkGetSwapchainImagesKHR(Device, Swapchain->Swapchain, &Swapchain->ImageCount, Swapchain->Images.data());

	return Swapchain;
}

void FVulkanSwapchain::Destroy()
{	
	VkDevice Device = Context->GetDevice();
	if (Swapchain != VK_NULL_HANDLE)
	{
		vkDestroySwapchainKHR(Device, Swapchain, nullptr);
	}
}

VkResult FVulkanSwapchain::Present(VkQueue InGfxQueue, VkQueue InPresentQueue, VkSemaphore InRenderFinishedSemaphore)
{
	VkSemaphore SignalSemaphores[] = { InRenderFinishedSemaphore };

	VkSwapchainKHR Swapchains[] = { Swapchain };

	VkPresentInfoKHR PresentInfo{};
	PresentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	PresentInfo.waitSemaphoreCount = 1;
	PresentInfo.pWaitSemaphores = SignalSemaphores;
	PresentInfo.swapchainCount = 1;
	PresentInfo.pSwapchains = Swapchains;
	PresentInfo.pImageIndices = &CurrentImageIndex;

	VkResult PresentResult = vkQueuePresentKHR(InPresentQueue, &PresentInfo);

	return VK_SUCCESS;
}

VkResult FVulkanSwapchain::AcquireNextImage(VkSemaphore InImageAcquiredSemaphore)
{
	VkDevice Device = Context->GetDevice();

	VkResult AcquireResult = vkAcquireNextImageKHR(
		Device,
		Swapchain,
		UINT64_MAX,
		InImageAcquiredSemaphore,
		VK_NULL_HANDLE,
		&CurrentImageIndex);
	return AcquireResult;
}
