#include "VulkanContext.h"
#include "VulkanHelpers.h"

#include "Config.h"

#include <array>
#include <vector>
#include <set>
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <unordered_map>

#define MAX_CONCURRENT_FRAME 2

static std::vector<const char*> GValidationLayers  =
{
	"VK_LAYER_KHRONOS_validation"
};

static std::vector<const char*> DeviceExtensions =
{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

#ifdef NDEBUG
static const bool GEnableValidationLayers = false;
#else
static const bool GEnableValidationLayers = true;
#endif

static std::unordered_map<GLFWwindow*, FVulkanContext*> RenderContextMap;

void FramebufferResizeCallback(GLFWwindow* InWindow, int InWidth, int InHeight)
{
	const auto Iter = RenderContextMap.find(InWindow);
	if (Iter == RenderContextMap.end())
	{
		return;
	}

	FVulkanContext* Context = Iter->second;
	if (Context == nullptr)
	{
		return;
	}

	Context->SetFramebufferResized(true);
}

VKAPI_ATTR VkBool32 VKAPI_CALL DebugMessengerCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT InMessageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT InMessageType,
	const VkDebugUtilsMessengerCallbackDataEXT* InCallbackData,
	void* InUserData)
{
	std::cerr << "Validation Layer: " << InCallbackData->pMessage << std::endl;

	return VK_FALSE;
}

FVulkanContext::FVulkanContext(GLFWwindow* InWindow)
	: Window(InWindow)
{
	RenderContextMap[InWindow] = this;

	glfwSetFramebufferSizeCallback(Window, FramebufferResizeCallback);

	CreateInstance();
	SetupDebugMessenger();
	CreateSurface();
	PickPhysicalDevice();
	CreateLogicalDevice();
	CreateSwapchain();
	CreateImageViews();
	CreateDepthResources();
	CreateCommandPool();
	CreateCommandBuffers();
	CreateRenderPass();
	CreateFramebuffers();
	CreateSyncObjects();
	CreateDescriptorPool();
}

FVulkanContext::~FVulkanContext()
{
	RenderContextMap.erase(Window);

	for (FVulkanObject* LiveObject : LiveObjects)
	{
		if (LiveObject != nullptr)
		{
			LiveObject->Destroy();
		}
	}

	for (FVulkanObject* LiveObject : LiveObjects)
	{
		if (LiveObject != nullptr)
		{
			delete LiveObject;
		}
	}

	CleanupSwapchain();

	vkDestroyDescriptorPool(Device, DescriptorPool, nullptr);
	vkDestroyRenderPass(Device, RenderPass, nullptr);
	vkDestroyCommandPool(Device, CommandPool, nullptr);

	for (size_t Idx = 0; Idx < MAX_CONCURRENT_FRAME; ++Idx)
	{
		vkDestroySemaphore(Device, ImageAvailableSemaphores[Idx], nullptr);
		vkDestroySemaphore(Device, RenderFinishedSemaphores[Idx], nullptr);
		vkDestroyFence(Device, Fences[Idx], nullptr);
	}

	vkDestroyDevice(Device, nullptr);

	if (GEnableValidationLayers)
	{
		auto Func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(Instance, "vkDestroyDebugUtilsMessengerEXT");
		if (Func != nullptr)
		{
			Func(Instance, DebugMessenger, nullptr);
		}
	}

	vkDestroySurfaceKHR(Instance, Surface, nullptr);
	vkDestroyInstance(Instance, nullptr);
}

void FVulkanContext::DestroyObject(FVulkanObject* InObject)
{
	if (InObject == nullptr)
	{
		return;
	}

	InObject->Destroy();

	for (int Idx = 0; Idx < LiveObjects.size(); ++Idx)
	{
		if (InObject == LiveObjects[Idx])
		{
			LiveObjects[Idx] = nullptr;
			delete InObject;
			break;
		}
	}

	std::remove_if(LiveObjects.begin(), LiveObjects.end(), [this](FVulkanObject* Object)
	{
		return Object == nullptr;
	});
}

void FVulkanContext::CreateInstance()
{
	if (GEnableValidationLayers && !Vk::SupportsValidationLayer(GValidationLayers))
	{
		throw std::runtime_error("validation layers requested, but not available.");
	}

	std::string ApplicationName;
	std::string EngineName;
	GConfig->Get("ApplicationName", ApplicationName);
	GConfig->Get("EngineName", EngineName);

	VkApplicationInfo ApplicationInfo{};
	ApplicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	ApplicationInfo.pApplicationName = ApplicationName.c_str();
	ApplicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	ApplicationInfo.pEngineName = EngineName.c_str();
	ApplicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	ApplicationInfo.apiVersion = VK_API_VERSION_1_0;

	uint32_t GLFWExtensionCount = 0;
	const char** GLFWExtensions = glfwGetRequiredInstanceExtensions(&GLFWExtensionCount);

	std::vector<const char*> Extensions(GLFWExtensions, GLFWExtensions + GLFWExtensionCount);
	Extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

	VkInstanceCreateInfo InstanceCI{};
	InstanceCI.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	InstanceCI.pApplicationInfo = &ApplicationInfo;
	InstanceCI.enabledExtensionCount = static_cast<uint32_t>(Extensions.size());
	InstanceCI.ppEnabledExtensionNames = Extensions.data();

	if (GEnableValidationLayers)
	{
		InstanceCI.enabledLayerCount = static_cast<uint32_t>(GValidationLayers.size());
		InstanceCI.ppEnabledLayerNames = GValidationLayers.data();

		VkDebugUtilsMessengerCreateInfoEXT DebugMessengerCI{};
		DebugMessengerCI.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		DebugMessengerCI.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		DebugMessengerCI.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		DebugMessengerCI.pfnUserCallback = DebugMessengerCallback;

		InstanceCI.pNext = &DebugMessengerCI;
	}
	else
	{
		InstanceCI.enabledLayerCount = 0;
		InstanceCI.pNext = nullptr;
	}

	if (vkCreateInstance(&InstanceCI, nullptr, &Instance) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create instance.");
	}
}

void FVulkanContext::SetupDebugMessenger()
{
	if (GEnableValidationLayers)
	{
		VkDebugUtilsMessengerCreateInfoEXT DebugMessengerCI{};
		DebugMessengerCI.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		DebugMessengerCI.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		DebugMessengerCI.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
		DebugMessengerCI.pfnUserCallback = DebugMessengerCallback;

		auto Func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(Instance, "vkCreateDebugUtilsMessengerEXT");
		if (Func != nullptr)
		{
			Func(Instance, &DebugMessengerCI, nullptr, &DebugMessenger);
		}
		else
		{
			throw std::runtime_error("Failed to set up debug messenger.");
		}
	}
}

void FVulkanContext::CreateSurface()
{
	if (glfwCreateWindowSurface(Instance, Window, nullptr, &Surface) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create window surface.");
	}
}

void FVulkanContext::PickPhysicalDevice()
{
	uint32_t DeviceCount = 0;
	vkEnumeratePhysicalDevices(Instance, &DeviceCount, nullptr);

	if (DeviceCount == 0)
	{
		throw std::runtime_error("Failed to find GPUs with Vulkan support.");
	}

	std::vector<VkPhysicalDevice> Devices(DeviceCount);
	vkEnumeratePhysicalDevices(Instance, &DeviceCount, Devices.data());

	for (VkPhysicalDevice Device : Devices)
	{
		if (Vk::IsDeviceSuitable(Device, Surface, DeviceExtensions))
		{
			PhysicalDevice = Device;
			break;
		}
	}

	if (PhysicalDevice == VK_NULL_HANDLE)
	{
		throw std::runtime_error("Failed to find a suitable GPU");
	}
}

void FVulkanContext::CreateLogicalDevice()
{
	uint32_t GraphicsFamily = -1;
	uint32_t PresentFamily = -1;

	Vk::FindQueueFamilies(PhysicalDevice, Surface, GraphicsFamily, PresentFamily);

	std::vector<VkDeviceQueueCreateInfo> QueueCIs{};
	std::set<uint32_t> UniqueQueueFamilies = { GraphicsFamily, PresentFamily };

	float QueuePriority = 1.0f;
	for (uint32_t QueueFamily : UniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo QueueCI{};
		QueueCI.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		QueueCI.queueFamilyIndex = QueueFamily;
		QueueCI.queueCount = 1;
		QueueCI.pQueuePriorities = &QueuePriority;
		QueueCIs.push_back(QueueCI);
	}

	VkPhysicalDeviceFeatures DeviceFeatures{};
	DeviceFeatures.samplerAnisotropy = VK_TRUE;
	DeviceFeatures.geometryShader = VK_TRUE;

	VkDeviceCreateInfo DeviceCI{};
	DeviceCI.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	DeviceCI.queueCreateInfoCount = static_cast<uint32_t>(QueueCIs.size());
	DeviceCI.pQueueCreateInfos = QueueCIs.data();

	DeviceCI.pEnabledFeatures = &DeviceFeatures;

	DeviceCI.enabledExtensionCount = static_cast<uint32_t>(DeviceExtensions.size());
	DeviceCI.ppEnabledExtensionNames = DeviceExtensions.data();

	if (GEnableValidationLayers)
	{
		DeviceCI.enabledLayerCount = static_cast<uint32_t>(GValidationLayers.size());
		DeviceCI.ppEnabledLayerNames = GValidationLayers.data();
	}
	else
	{
		DeviceCI.enabledLayerCount = 0;
	}

	if (vkCreateDevice(PhysicalDevice, &DeviceCI, nullptr, &Device) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create logical device.");
	}

	vkGetDeviceQueue(Device, GraphicsFamily, 0, &GfxQueue);
	vkGetDeviceQueue(Device, PresentFamily, 0, &PresentQueue);
}

void FVulkanContext::CreateSwapchain()
{
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
		glfwGetFramebufferSize(Window, &Width, &Height);

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

	if (vkCreateSwapchainKHR(Device, &SwapchainCI, nullptr, &Swapchain) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create swap chain.");
	}

	uint32_t SwapchainImageCount;
	vkGetSwapchainImagesKHR(Device, Swapchain, &SwapchainImageCount, nullptr);

	SwapchainImages.resize(SwapchainImageCount);
	vkGetSwapchainImagesKHR(Device, Swapchain, &SwapchainImageCount, SwapchainImages.data());

	SwapchainImageFormat = ChoosenFormat.format;
	SwapchainExtent = ChoosenSwapchainExtent;
}

void FVulkanContext::CreateImageViews()
{
	SwapchainImageViews.resize(SwapchainImages.size());

	for (uint32_t Idx = 0; Idx < SwapchainImages.size(); ++Idx)
	{
		VkImageViewCreateInfo ImageViewCI{};
		ImageViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		ImageViewCI.image = SwapchainImages[Idx];
		ImageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
		ImageViewCI.format = SwapchainImageFormat;
		ImageViewCI.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		ImageViewCI.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		ImageViewCI.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		ImageViewCI.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		ImageViewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		ImageViewCI.subresourceRange.baseMipLevel = 0;
		ImageViewCI.subresourceRange.levelCount = 1;
		ImageViewCI.subresourceRange.baseArrayLayer = 0;
		ImageViewCI.subresourceRange.layerCount = 1;

		if (vkCreateImageView(Device, &ImageViewCI, nullptr, &SwapchainImageViews[Idx]) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create image views.");
		}
	}
}

void FVulkanContext::CreateFramebuffers()
{
	SwapchainFramebuffers.resize(SwapchainImageViews.size());

	for (size_t Idx = 0; Idx < SwapchainImageViews.size(); ++Idx)
	{
		std::array<VkImageView, 2> Attachments =
		{
			SwapchainImageViews[Idx],
			DepthImageView
		};

		VkFramebufferCreateInfo FramebufferCI{};
		FramebufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		FramebufferCI.renderPass = RenderPass;
		FramebufferCI.attachmentCount = static_cast<uint32_t>(Attachments.size());
		FramebufferCI.pAttachments = Attachments.data();
		FramebufferCI.width = SwapchainExtent.width;
		FramebufferCI.height = SwapchainExtent.height;
		FramebufferCI.layers = 1;
		FramebufferCI.pNext = nullptr;

		if (vkCreateFramebuffer(Device, &FramebufferCI, nullptr, &SwapchainFramebuffers[Idx]) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create framebuffer.");
		}
	}
}

void FVulkanContext::CreateRenderPass()
{
	VkAttachmentDescription ColorAttachmentDesc{};
	ColorAttachmentDesc.format = SwapchainImageFormat;
	ColorAttachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
	ColorAttachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	ColorAttachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	ColorAttachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	ColorAttachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	ColorAttachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	ColorAttachmentDesc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentDescription DepthAttachmentDesc{};
	DepthAttachmentDesc.format = Vk::FindDepthFormat(PhysicalDevice);
	DepthAttachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
	DepthAttachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	DepthAttachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	DepthAttachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	DepthAttachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	DepthAttachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	DepthAttachmentDesc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference ColorAttachmentRef{};
	ColorAttachmentRef.attachment = 0;
	ColorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference DepthAttachmentRef{};
	DepthAttachmentRef.attachment = 1;
	DepthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription SubpassDesc{};
	SubpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	SubpassDesc.colorAttachmentCount = 1;
	SubpassDesc.pColorAttachments = &ColorAttachmentRef;
	SubpassDesc.pDepthStencilAttachment = &DepthAttachmentRef;

	VkSubpassDependency SubpassDependency{};
	SubpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	SubpassDependency.dstSubpass = 0;
	SubpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	SubpassDependency.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	SubpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	SubpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	std::array<VkAttachmentDescription, 2> Attachments = { ColorAttachmentDesc, DepthAttachmentDesc };

	VkRenderPassCreateInfo RenderPassCI{};
	RenderPassCI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	RenderPassCI.attachmentCount = static_cast<uint32_t>(Attachments.size());
	RenderPassCI.pAttachments = Attachments.data();
	RenderPassCI.subpassCount = 1;
	RenderPassCI.pSubpasses = &SubpassDesc;
	RenderPassCI.dependencyCount = 1;
	RenderPassCI.pDependencies = &SubpassDependency;

	if (vkCreateRenderPass(Device, &RenderPassCI, nullptr, &RenderPass) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create render pass.");
	}
}

void FVulkanContext::CreateCommandPool()
{
	uint32_t GraphicsFamily = -1;
	uint32_t PresentFamily = -1;
	Vk::FindQueueFamilies(PhysicalDevice, Surface, GraphicsFamily, PresentFamily);

	VkCommandPoolCreateInfo CommandPoolCI{};
	CommandPoolCI.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	CommandPoolCI.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	CommandPoolCI.queueFamilyIndex = GraphicsFamily;

	if (vkCreateCommandPool(Device, &CommandPoolCI, nullptr, &CommandPool) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create graphics command pool.");
	}
}

void FVulkanContext::CreateCommandBuffers()
{
	CommandBuffers.resize(MAX_CONCURRENT_FRAME);

	VkCommandBufferAllocateInfo CommandBufferAllocInfo{};
	CommandBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	CommandBufferAllocInfo.commandPool = CommandPool;
	CommandBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	CommandBufferAllocInfo.commandBufferCount = static_cast<uint32_t>(CommandBuffers.size());

	if (vkAllocateCommandBuffers(Device, &CommandBufferAllocInfo, CommandBuffers.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate command buffers.");
	}
}

void FVulkanContext::CreateSyncObjects()
{
	ImageAvailableSemaphores.resize(MAX_CONCURRENT_FRAME);
	RenderFinishedSemaphores.resize(MAX_CONCURRENT_FRAME);
	Fences.resize(MAX_CONCURRENT_FRAME);

	VkSemaphoreCreateInfo SemaphoreCI{};
	SemaphoreCI.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo FenceCI{};
	FenceCI.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	FenceCI.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t Idx = 0; Idx < MAX_CONCURRENT_FRAME; ++Idx)
	{
		if (vkCreateSemaphore(Device, &SemaphoreCI, nullptr, &ImageAvailableSemaphores[Idx]) != VK_SUCCESS ||
			vkCreateSemaphore(Device, &SemaphoreCI, nullptr, &RenderFinishedSemaphores[Idx]) != VK_SUCCESS ||
			vkCreateFence(Device, &FenceCI, nullptr, &Fences[Idx]) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create synchronization objects for a frame.");
		}
	}
}

void FVulkanContext::CreateDescriptorPool()
{
	std::vector<VkDescriptorPoolSize> PoolSizes =
	{
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 100 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 100 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 100 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 100 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 100 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 100 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 100 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 100 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 100 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 100 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 100 },
	};

	VkDescriptorPoolCreateInfo DescriptorPoolCI{};
	DescriptorPoolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	DescriptorPoolCI.poolSizeCount = static_cast<uint32_t>(PoolSizes.size());
	DescriptorPoolCI.pPoolSizes = PoolSizes.data();
	DescriptorPoolCI.maxSets = 1000U;
	DescriptorPoolCI.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

	if (vkCreateDescriptorPool(Device, &DescriptorPoolCI, nullptr, &DescriptorPool) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create descriptor pool.");
	}
}

void FVulkanContext::CreateDepthResources()
{
	VkFormat DepthFormat = Vk::FindDepthFormat(PhysicalDevice);

	Vk::CreateImage(
		PhysicalDevice,
		Device,
		SwapchainExtent.width,
		SwapchainExtent.height,
		1,
		DepthFormat,
		VK_IMAGE_TYPE_2D,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		DepthImage,
		DepthImageMemory);

	DepthImageView = Vk::CreateImageView(
		Device,
		DepthImage,
		DepthFormat,
		VK_IMAGE_VIEW_TYPE_2D,
		VK_IMAGE_ASPECT_DEPTH_BIT);
}

void FVulkanContext::CleanupSwapchain()
{
	vkDestroyImageView(Device, DepthImageView, nullptr);
	vkDestroyImage(Device, DepthImage, nullptr);
	vkFreeMemory(Device, DepthImageMemory, nullptr);

	for (VkFramebuffer Framebuffer : SwapchainFramebuffers)
	{
		vkDestroyFramebuffer(Device, Framebuffer, nullptr);
	}

	for (VkImageView ImageView : SwapchainImageViews)
	{
		vkDestroyImageView(Device, ImageView, nullptr);
	}

	vkDestroySwapchainKHR(Device, Swapchain, nullptr);
}

void FVulkanContext::RecreateSwapchain()
{
	int Width = 0, Height = 0;
	glfwGetFramebufferSize(Window, &Width, &Height);

	while (Width == 0 || Height == 0)
	{
		glfwGetFramebufferSize(Window, &Width, &Height);
		glfwWaitEvents();
	}

	vkDeviceWaitIdle(Device);

	CleanupSwapchain();

	CreateSwapchain();
	CreateImageViews();
	CreateDepthResources();
	CreateFramebuffers();
}

void FVulkanContext::BeginRender()
{
	vkWaitForFences(Device, 1, &Fences[CurrentFrame], VK_TRUE, UINT64_MAX);

	VkResult AcquireResult = vkAcquireNextImageKHR(
		Device,
		Swapchain,
		UINT64_MAX,
		ImageAvailableSemaphores[CurrentFrame],
		VK_NULL_HANDLE,
		&CurrentImageIndex);

	if (AcquireResult == VK_ERROR_OUT_OF_DATE_KHR)
	{
		RecreateSwapchain();
		return;
	}
	else if (AcquireResult != VK_SUCCESS && AcquireResult != VK_SUBOPTIMAL_KHR)
	{
		throw std::runtime_error("Failed to acquire swap chain image.");
	}

	vkResetFences(Device, 1, &Fences[CurrentFrame]);

	vkResetCommandBuffer(CommandBuffers[CurrentFrame], 0);

	VkCommandBufferBeginInfo CommandBufferBeginInfo{};
	CommandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	if (vkBeginCommandBuffer(CommandBuffers[CurrentFrame], &CommandBufferBeginInfo) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to begin recording command buffer.");
	}

	VkRenderPassBeginInfo RenderPassBeginInfo{};
	RenderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	RenderPassBeginInfo.renderPass = RenderPass;
	RenderPassBeginInfo.framebuffer = SwapchainFramebuffers[CurrentImageIndex];
	RenderPassBeginInfo.renderArea.offset = { 0, 0 };
	RenderPassBeginInfo.renderArea.extent = SwapchainExtent;

	std::array<VkClearValue, 2> ClearValues{};
	ClearValues[0].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
	ClearValues[1].depthStencil = { 1.0f, 0 };
	RenderPassBeginInfo.clearValueCount = 2;
	RenderPassBeginInfo.pClearValues = ClearValues.data();

	vkCmdBeginRenderPass(CommandBuffers[CurrentFrame], &RenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void FVulkanContext::EndRender()
{
	vkCmdEndRenderPass(CommandBuffers[CurrentFrame]);

	if (vkEndCommandBuffer(CommandBuffers[CurrentFrame]) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to record command buffer.");
	}

	VkSemaphore WaitSemaphores[] = { ImageAvailableSemaphores[CurrentFrame] };
	VkPipelineStageFlags WaitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	VkSemaphore SignalSemaphores[] = { RenderFinishedSemaphores[CurrentFrame] };

	VkSubmitInfo SubmitInfo{};
	SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	SubmitInfo.waitSemaphoreCount = 1;
	SubmitInfo.pWaitSemaphores = WaitSemaphores;
	SubmitInfo.pWaitDstStageMask = WaitStages;
	SubmitInfo.commandBufferCount = 1;
	SubmitInfo.pCommandBuffers = &CommandBuffers[CurrentFrame];
	SubmitInfo.signalSemaphoreCount = 1;
	SubmitInfo.pSignalSemaphores = SignalSemaphores;

	if (vkQueueSubmit(GfxQueue, 1, &SubmitInfo, Fences[CurrentFrame]) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to submit draw command buffer.");
	}

	VkSwapchainKHR Swapchains[] = { Swapchain };

	VkPresentInfoKHR PresentInfo{};
	PresentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	PresentInfo.waitSemaphoreCount = 1;
	PresentInfo.pWaitSemaphores = SignalSemaphores;
	PresentInfo.swapchainCount = 1;
	PresentInfo.pSwapchains = Swapchains;
	PresentInfo.pImageIndices = &CurrentImageIndex;

	VkResult PresentResult = vkQueuePresentKHR(PresentQueue, &PresentInfo);
	if (PresentResult == VK_ERROR_OUT_OF_DATE_KHR || PresentResult == VK_SUBOPTIMAL_KHR || bFramebufferResized)
	{
		bFramebufferResized = false;
		RecreateSwapchain();
	}
	else if (PresentResult != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to present swap chain image.");
	}

	CurrentFrame = (CurrentFrame + 1) % MAX_CONCURRENT_FRAME;
}
