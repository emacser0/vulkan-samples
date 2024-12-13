#include "VulkanContext.h"
#include "VulkanHelpers.h"
#include "VulkanImage.h"
#include "VulkanSwapchain.h"
#include "VulkanViewport.h"
#include "VulkanFramebuffer.h"
#include "VulkanRenderPass.h"
#include "VulkanRenderer.h"
#include "VulkanMeshRenderer.h"
#include "VulkanUIRenderer.h"

#include "Config.h"

#include <array>
#include <vector>
#include <set>
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <unordered_map>

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
	, Instance(VK_NULL_HANDLE)
	, DebugMessenger(VK_NULL_HANDLE)
	, Surface(VK_NULL_HANDLE)
	, PhysicalDevice(VK_NULL_HANDLE)
	, Device(VK_NULL_HANDLE)
	, Swapchain(nullptr)
	, DepthImage(nullptr)
	, SkyRenderer(nullptr)
	, ShadowRenderer(nullptr)
	, MeshRenderer(nullptr)
	, UIRenderer(nullptr)
{
	RenderContextMap[InWindow] = this;

	glfwSetFramebufferSizeCallback(Window, FramebufferResizeCallback);

	CreateInstance();
	SetupDebugMessenger();
	CreateSurface();
	PickPhysicalDevice();
	CreateLogicalDevice();
	CreateSwapchain();
	CreateCommandPool();
	CreateCommandBuffers();
	CreateFramebuffers();
	CreateSyncObjects();
	CreateDescriptorPool();
	CreateRenderers();
}

FVulkanContext::~FVulkanContext()
{
	RenderContextMap.erase(Window);

	for (int Idx = 0; Idx < LiveObjects.size(); ++Idx)
	{
		if (LiveObjects[Idx] != nullptr)
		{
			LiveObjects[Idx]->Destroy();
			delete LiveObjects[Idx];
			LiveObjects[Idx] = nullptr;
		}
	}

	CleanupSwapchain();

	vkDestroyDescriptorPool(Device, DescriptorPool, nullptr);
	vkDestroyCommandPool(Device, CommandPool, nullptr);

	for (size_t Idx = 0; Idx < GetMaxConcurrentFrames(); ++Idx)
	{
		vkDestroySemaphore(Device, ImageAcquiredSemaphores[Idx], nullptr);
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

void FVulkanContext::WaitIdle()
{
	vkDeviceWaitIdle(Device);
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
}

bool FVulkanContext::IsValidObject(FVulkanObject* InObject)
{
	if (InObject == nullptr)
	{
		return false;
	}

	return std::find(LiveObjects.begin(), LiveObjects.end(), InObject) != LiveObjects.end();
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

	VK_ASSERT(vkCreateInstance(&InstanceCI, nullptr, &Instance));
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

	VK_ASSERT(vkCreateDevice(PhysicalDevice, &DeviceCI, nullptr, &Device));

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

	Swapchain = FVulkanSwapchain::Create(this, SwapchainCI);
}

void FVulkanContext::CreateRenderers()
{
	MeshRenderer = CreateObject<FVulkanMeshRenderer>();
	UIRenderer = CreateObject<FVulkanUIRenderer>();
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

	VK_ASSERT(vkCreateCommandPool(Device, &CommandPoolCI, nullptr, &CommandPool));
}

void FVulkanContext::CreateCommandBuffers()
{
	CommandBuffers.resize(GetMaxConcurrentFrames());

	VkCommandBufferAllocateInfo CommandBufferAllocInfo{};
	CommandBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	CommandBufferAllocInfo.commandPool = CommandPool;
	CommandBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	CommandBufferAllocInfo.commandBufferCount = static_cast<uint32_t>(CommandBuffers.size());

	VK_ASSERT(vkAllocateCommandBuffers(Device, &CommandBufferAllocInfo, CommandBuffers.data()));
}

void FVulkanContext::CreateSyncObjects()
{
	uint32_t MaxConcurrentFrames = GetMaxConcurrentFrames();

	ImageAcquiredSemaphores.resize(MaxConcurrentFrames);
	RenderFinishedSemaphores.resize(MaxConcurrentFrames);
	Fences.resize(MaxConcurrentFrames);

	VkSemaphoreCreateInfo SemaphoreCI{};
	SemaphoreCI.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo FenceCI{};
	FenceCI.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	FenceCI.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t Idx = 0; Idx < MaxConcurrentFrames; ++Idx)
	{
		if (vkCreateSemaphore(Device, &SemaphoreCI, nullptr, &ImageAcquiredSemaphores[Idx]) != VK_SUCCESS ||
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

	VK_ASSERT(vkCreateDescriptorPool(Device, &DescriptorPoolCI, nullptr, &DescriptorPool));
}

void FVulkanContext::CreateViewport()
{
	Viewport = FVulkanViewport::Create(this, Window);
}

void FVulkanContext::CleanupSwapchain()
{
	if (IsValidObject(DepthImage))
	{
		DestroyObject(DepthImage);
		DepthImage = nullptr;
	}

	for (FVulkanFramebuffer* Framebuffer : SwapchainFramebuffers)
	{
		if (IsValidObject(Framebuffer))
		{
			DestroyObject(Framebuffer);
		}
	}
	SwapchainFramebuffers.clear();

	if (IsValidObject(Swapchain))
	{
		DestroyObject(Swapchain);
		Swapchain = nullptr;
	}
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

	Viewport->Recreate();

	for (FVulkanRenderer* Renderer : Renderers)
	{
		if (Renderer != nullptr)
		{
			Renderer->OnRecreateSwapchain();
		}
	}
}

void FVulkanContext::Render()
{
	BeginRender();

	for (FVulkanRenderer* Renderer : Renderers)
	{
		if (Renderer != nullptr)
		{
			Renderer->Render();
		}
	}

	EndRender();
}

void FVulkanContext::BeginRender()
{
	vkWaitForFences(Device, 1, &Fences[CurrentFrame], VK_TRUE, UINT64_MAX);

	VkResult AcquireResult = Swapchain->AcquireNextImage(ImageAcquiredSemaphores[CurrentFrame]);
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

	VkCommandBuffer CommandBuffer = CommandBuffers[CurrentFrame];

	vkResetCommandBuffer(CommandBuffer, 0);

	VkCommandBufferBeginInfo CommandBufferBeginInfo{};
	CommandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	VK_ASSERT(vkBeginCommandBuffer(CommandBuffer, &CommandBufferBeginInfo));
}

void FVulkanContext::EndRender()
{
	VkCommandBuffer CommandBuffer = CommandBuffers[CurrentFrame];

	VK_ASSERT(vkEndCommandBuffer(CommandBuffer));

	VkSemaphore WaitSemaphores[] = { ImageAcquiredSemaphores[CurrentFrame] };
	VkPipelineStageFlags WaitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	VkSemaphore SignalSemaphores[] = { RenderFinishedSemaphores[CurrentFrame] };

	VkSubmitInfo SubmitInfo{};
	SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	SubmitInfo.waitSemaphoreCount = 1;
	SubmitInfo.pWaitSemaphores = WaitSemaphores;
	SubmitInfo.pWaitDstStageMask = WaitStages;
	SubmitInfo.commandBufferCount = 1;
	SubmitInfo.pCommandBuffers = &CommandBuffer;
	SubmitInfo.signalSemaphoreCount = 1;
	SubmitInfo.pSignalSemaphores = SignalSemaphores;

	VK_ASSERT(vkQueueSubmit(GfxQueue, 1, &SubmitInfo, Fences[CurrentFrame]));

	VkResult PresentResult = Swapchain->Present(GfxQueue, PresentQueue, RenderFinishedSemaphores[CurrentFrame]);
	if (PresentResult == VK_ERROR_OUT_OF_DATE_KHR || PresentResult == VK_SUBOPTIMAL_KHR || bFramebufferResized)
	{
		bFramebufferResized = false;
		RecreateSwapchain();
	}
	else if (PresentResult != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to present swap chain image.");
	}

	CurrentFrame = (CurrentFrame + 1) % GetMaxConcurrentFrames();
}
