#include "VulkanViewport.h"
#include "VulkanContext.h"
#include "VulkanHelpers.h"
#include "VulkanImage.h"
#include "VulkanFramebuffer.h"
#include "VulkanSwapchain.h"

#include <algorithm>

FVulkanViewport::FVulkanViewport(FVulkanContext* InContext)
	: FVulkanObject(InContext)
	, Swapchain(nullptr)
	, DepthImage(nullptr)
{
}

FVulkanViewport::~FVulkanViewport()
{
}

FVulkanViewport* FVulkanViewport::Create(FVulkanContext* InContext, GLFWwindow* InWindow)
{
	FVulkanViewport* Viewport = InContext->CreateObject<FVulkanViewport>();
	Viewport->CreateSwapchain(InWindow);
	Viewport->CreateDepthImage();

	return Viewport;
}

void FVulkanViewport::Destroy()
{
	Cleanup();
}

void FVulkanViewport::CreateSwapchain(GLFWwindow* InWindow)
{	
	VkPhysicalDevice PhysicalDevice = Context->GetPhysicalDevice();
	VkSurfaceKHR Surface = Context->GetSurface();

	VkSurfaceCapabilitiesKHR Capabilities;
	std::vector<VkSurfaceFormatKHR> Formats;
	std::vector<VkPresentModeKHR> PresentModes;
	Vk::QuerySwapchainSupport(PhysicalDevice, Surface, Capabilities, Formats, PresentModes);

	assert(Formats.size() > 0);
	assert(PresentModes.size() > 0);

	VkSurfaceFormatKHR ChoosenFormat = Formats[0];
	for (VkSurfaceFormatKHR Format : Formats)
	{
		if (Format.format == VK_FORMAT_B8G8R8A8_SRGB && Format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			ChoosenFormat = Format;
			break;
		}
	}

	VkPresentModeKHR ChoosenPresentMode = VK_PRESENT_MODE_FIFO_KHR;
	for (VkPresentModeKHR PresentMode : PresentModes)
	{
		if (PresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			ChoosenPresentMode = PresentMode;
			break;
		}
	}

	VkExtent2D ChoosenSwapchainExtent;
	if (Capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		ChoosenSwapchainExtent = Capabilities.currentExtent;
	}
	else
	{
		int Width, Height;
		glfwGetFramebufferSize(InWindow, &Width, &Height);

		ChoosenSwapchainExtent =
		{
			static_cast<uint32_t>(Width),
			static_cast<uint32_t>(Height)
		};

		ChoosenSwapchainExtent.width = std::clamp(
			ChoosenSwapchainExtent.width,
			Capabilities.minImageExtent.width,
			Capabilities.maxImageExtent.width);
		ChoosenSwapchainExtent.height = std::clamp(
			ChoosenSwapchainExtent.height,
			Capabilities.minImageExtent.height,
			Capabilities.maxImageExtent.height);
	}

	uint32_t ChoosenImageCount = std::clamp(
		Capabilities.minImageCount + 1,
		1U,
		Capabilities.maxImageCount);

	VkSwapchainCreateInfoKHR SwapchainCI{};
	SwapchainCI.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	SwapchainCI.surface = Surface;

	SwapchainCI.minImageCount = ChoosenImageCount;
	SwapchainCI.imageFormat = ChoosenFormat.format;
	SwapchainCI.imageColorSpace = ChoosenFormat.colorSpace;
	SwapchainCI.imageExtent = ChoosenSwapchainExtent;
	SwapchainCI.imageArrayLayers = 1;
	SwapchainCI.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	uint32_t GraphicsFamily = -1;
	uint32_t PresentFamily = -1;
	Vk::FindQueueFamilies(PhysicalDevice, Surface, GraphicsFamily, PresentFamily);

	uint32_t QueueFamilyIndices[] = { GraphicsFamily, PresentFamily };

	if (GraphicsFamily == PresentFamily)
	{
		SwapchainCI.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}
	else
	{
		SwapchainCI.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		SwapchainCI.queueFamilyIndexCount = 2;
		SwapchainCI.pQueueFamilyIndices = QueueFamilyIndices;
	}

	SwapchainCI.preTransform = Capabilities.currentTransform;
	SwapchainCI.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	SwapchainCI.presentMode = ChoosenPresentMode;
	SwapchainCI.clipped = VK_TRUE;

	Swapchain = FVulkanSwapchain::Create(Context, SwapchainCI);
}

void FVulkanViewport::CreateDepthImage()
{
	VkPhysicalDevice PhysicalDevice = Context->GetPhysicalDevice();

	VkFormat DepthFormat = Vk::FindDepthFormat(PhysicalDevice);

	if (Context->IsValidObject(DepthImage))
	{
		Context->DestroyObject(DepthImage);
		DepthImage = nullptr;
	}

	VkExtent2D SwapchainExtent = Swapchain->GetExtent();

	DepthImage = Context->CreateObject<FVulkanImage>();
	DepthImage->CreateImage(
		{ SwapchainExtent.width, SwapchainExtent.height, 1 },
		1,
		1,
		DepthFormat,
		VK_IMAGE_TYPE_2D,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	DepthImage->CreateView(
		VK_IMAGE_VIEW_TYPE_2D,
		VK_IMAGE_ASPECT_DEPTH_BIT);
}

void FVulkanViewport::Recreate()
{
	Cleanup();
	CreateSwapchain(Context->GetWindow());
	CreateDepthImage();
}

void FVulkanViewport::Cleanup()
{
	if (Context->IsValidObject(DepthImage))
	{
		Context->DestroyObject(DepthImage);
		DepthImage = nullptr;
	}

	if (Context->IsValidObject(Swapchain))
	{
		Context->DestroyObject(Swapchain);
		Swapchain = nullptr;
	}
}
