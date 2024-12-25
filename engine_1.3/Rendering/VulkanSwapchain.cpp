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
	Swapchain->ImageViews.resize(Swapchain->ImageCount);
	vkGetSwapchainImagesKHR(Device, Swapchain->Swapchain, &Swapchain->ImageCount, Swapchain->Images.data());

	for (uint32_t Idx = 0; Idx < Swapchain->GetImageCount(); ++Idx)
	{
		VkImageViewCreateInfo ImageViewCI{};
		ImageViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		ImageViewCI.image = Swapchain->Images[Idx];
		ImageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
		ImageViewCI.format = Swapchain->GetFormat();
		ImageViewCI.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		ImageViewCI.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		ImageViewCI.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		ImageViewCI.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		ImageViewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		ImageViewCI.subresourceRange.baseMipLevel = 0;
		ImageViewCI.subresourceRange.levelCount = 1;
		ImageViewCI.subresourceRange.baseArrayLayer = 0;
		ImageViewCI.subresourceRange.layerCount = 1;

		VK_ASSERT(vkCreateImageView(Device, &ImageViewCI, nullptr, &Swapchain->ImageViews[Idx]));
	}

	return Swapchain;
}

void FVulkanSwapchain::Destroy()
{	
	VkDevice Device = Context->GetDevice();
	if (Swapchain != VK_NULL_HANDLE)
	{
		vkDestroySwapchainKHR(Device, Swapchain, nullptr);
	}

	for (VkImageView ImageView : ImageViews)
	{
		if (ImageView != VK_NULL_HANDLE)
		{
			vkDestroyImageView(Device, ImageView, nullptr);
		}
	}
	ImageViews.clear();
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
