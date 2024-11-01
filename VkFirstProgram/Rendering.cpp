#include "Rendering.h"
#include "Config.h"

static const char* GValidationLayers[] =
{
	"VK_LAYER_KHRONOS_validation"
};
static const uint32_t GValidationLayerCount = sizeof(GValidationLayers) / sizeof(const char*);

static const char* GDeviceExtensions[] =
{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};
static const uint32_t GDeviceExtensionCount = sizeof(GDeviceExtensions) / sizeof(const char*);

#ifdef NDEBUG
static const bool GEnableValidationLayers = false;
#else
static const bool GEnableValidationLayers = true;
#endif

GLFWwindow* GWindow;

VkInstance GInstance;
VkDebugUtilsMessengerEXT GDebugMessenger;
VkSurfaceKHR GSurface;

VkPhysicalDevice GPhysicalDevice = VK_NULL_HANDLE;
VkDevice GDevice;

VkQueue GGraphicsQueue;
VkQueue GPresentQueue;

VkSwapchainKHR GSwapchain;
std::vector<VkImage> GSwapchainImages;
VkFormat GSwapchainImageFormat;
VkExtent2D GSwapchainExtent;
std::vector<VkImageView> GSwapchainImageViews;
std::vector<VkFramebuffer> GSwapchainFramebuffers;

VkRenderPass GRenderPass;
VkPipelineLayout GPipelineLayout;
VkPipeline GPipeline;
VkCommandPool GCommandPool;

VkBuffer GVertexBuffer;
VkDeviceMemory GVertexBufferMemory;
VkBuffer GIndexBuffer;
VkDeviceMemory GIndexBufferMemory;

std::vector<VkCommandBuffer> GCommandBuffers;

std::vector<VkSemaphore> GImageAvailableSemaphores;
std::vector<VkSemaphore> GRenderFinishedSemaphores;
std::vector<VkFence> GFences;

bool GFramebufferResized = false;

uint32_t GCurrentFrame = 0;

const std::vector<FVertex> GVertices =
{
	{ { -0.5f, 0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
	{ { 0.0f, -0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f } },
	{ { 0.5f, 0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
};

const std::vector<uint16_t> GIndices =
{
	0, 1, 2
};

void FramebufferResizeCallback(GLFWwindow* InWindow, int InWidth, int InHeight)
{
	GFramebufferResized = true;
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

bool SupportsValidationLayer()
{
	uint32_t LayerCount;
	vkEnumerateInstanceLayerProperties(&LayerCount, nullptr);

	std::vector<VkLayerProperties> AvailableLayers(LayerCount);
	vkEnumerateInstanceLayerProperties(&LayerCount, AvailableLayers.data());

	for (const char* LayerName : GValidationLayers)
	{
		bool bLayerFound = false;

		for (const VkLayerProperties& LayerProperties : AvailableLayers)
		{
			if (strcmp(LayerName, LayerProperties.layerName) == 0)
			{
				bLayerFound = true;
				break;
			}
		}

		if (!bLayerFound)
		{
			return false;
		}
	}

	return true;
}

bool DeviceSupportsExtensions(VkPhysicalDevice InDevice)
{
	uint32_t ExtensionCount;
	vkEnumerateDeviceExtensionProperties(InDevice, nullptr, &ExtensionCount, nullptr);

	if (ExtensionCount == 0)
	{
		return GDeviceExtensionCount == 0;
	}

	std::vector<VkExtensionProperties> AvailableExtensions(ExtensionCount);
	vkEnumerateDeviceExtensionProperties(InDevice, nullptr, &ExtensionCount, AvailableExtensions.data());

	for (const char* RequiredExtension : GDeviceExtensions)
	{
		bool bContainsExtension = false;
		for (const VkExtensionProperties& Extension : AvailableExtensions)
		{
			if (strcmp(Extension.extensionName, RequiredExtension) == 0)
			{
				bContainsExtension = true;
				break;
			}
		}

		if (bContainsExtension == false)
		{
			return false;
		}
	}

	return true;
}

void FindQueueFamilies(VkPhysicalDevice InDevice, uint32_t& OutGraphicsFamily, uint32_t& OutPresentFamily)
{
	OutGraphicsFamily = -1;
	OutPresentFamily = -1;

	uint32_t QueueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(InDevice, &QueueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> QueueFamilies(QueueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(InDevice, &QueueFamilyCount, QueueFamilies.data());

	for (uint32_t Idx = 0; Idx < QueueFamilyCount; ++Idx)
	{
		if (QueueFamilies[Idx].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			OutGraphicsFamily = Idx;
		}

		VkBool32 PresentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(InDevice, Idx, GSurface, &PresentSupport);

		if (PresentSupport)
		{
			OutPresentFamily = Idx;
		}

		if (OutGraphicsFamily != -1 && OutPresentFamily != -1)
		{
			break;
		}
	}
}

void QuerySwapchainSupport(
	VkPhysicalDevice InDevice,
	VkSurfaceCapabilitiesKHR& OutCapabilities,
	std::vector<VkSurfaceFormatKHR>& OutFormats,
	std::vector<VkPresentModeKHR>& OutPresentModes)
{
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(InDevice, GSurface, &OutCapabilities);

	uint32_t FormatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(InDevice, GSurface, &FormatCount, nullptr);
	
	if (FormatCount > 0)
	{
		OutFormats.resize(FormatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(InDevice, GSurface, &FormatCount, OutFormats.data());
	}

	uint32_t PresentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(InDevice, GSurface, &PresentModeCount, nullptr);

	if (PresentModeCount > 0)
	{
		OutPresentModes.resize(PresentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(InDevice, GSurface, &PresentModeCount, OutPresentModes.data());
	}
}

bool IsDeviceSuitable(VkPhysicalDevice InDevice)
{
	uint32_t GraphicsFamily = -1;
	uint32_t PresentFamily = -1;

	FindQueueFamilies(InDevice, GraphicsFamily, PresentFamily);

	bool bExtensionsSupported = DeviceSupportsExtensions(InDevice);

	bool bSwapchainAdequate = false;
	if (bExtensionsSupported)
	{
		VkSurfaceCapabilitiesKHR Capabilities;
		std::vector<VkSurfaceFormatKHR> Formats;
		std::vector<VkPresentModeKHR> PresentModes;

		QuerySwapchainSupport(InDevice, Capabilities, Formats, PresentModes);
		bSwapchainAdequate = !Formats.empty() && !PresentModes.empty();
	}

	return GraphicsFamily != -1 && PresentFamily != -1 && bExtensionsSupported && bSwapchainAdequate;
}

bool ReadFile(const std::string& InFilename, std::vector<char>& OutBytes)
{
	std::ifstream File(InFilename, std::ios::ate | std::ios::binary);
	if (File.is_open() == false)
	{
		return false;
	}

	size_t FileSize = (size_t)File.tellg();
	OutBytes.resize(FileSize);

	File.seekg(0);
	File.read(OutBytes.data(), FileSize);

	File.close();

	return true;
}

VkShaderModule CreateShaderModule(const std::vector<char>& InCode)
{
	VkShaderModuleCreateInfo ShaderModuleCI{};
	ShaderModuleCI.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	ShaderModuleCI.codeSize = InCode.size();
	ShaderModuleCI.pCode = reinterpret_cast<const uint32_t*>(InCode.data());

	VkShaderModule ShaderModule;
	if (vkCreateShaderModule(GDevice, &ShaderModuleCI, nullptr, &ShaderModule) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create shader module.");
	}

	return ShaderModule;
}

uint32_t FindMemoryType(uint32_t InTypeFilter, VkMemoryPropertyFlags InProperties)
{
	VkPhysicalDeviceMemoryProperties MemoryProperties;
	vkGetPhysicalDeviceMemoryProperties(GPhysicalDevice, &MemoryProperties);

	for (uint32_t Idx = 0; Idx < MemoryProperties.memoryTypeCount; ++Idx)
	{
		if ((InTypeFilter & (1 << Idx)) && (MemoryProperties.memoryTypes[Idx].propertyFlags & InProperties) == InProperties)
		{
			return Idx;
		}
	}

	throw std::runtime_error("Failed to find suitable memory type");
	return -1;
}

void CreateBuffer(
	VkDeviceSize InSize,
	VkBufferUsageFlags InUsage,
	VkMemoryPropertyFlags InProperties,
	VkBuffer& OutBuffer,
	VkDeviceMemory& OutMemory)
{
	VkBufferCreateInfo BufferCI{};
	BufferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	BufferCI.size = InSize;
	BufferCI.usage = InUsage;
	BufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(GDevice, &BufferCI, nullptr, &OutBuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create buffer.");
	}

	VkMemoryRequirements MemoryReqs{};
	vkGetBufferMemoryRequirements(GDevice, OutBuffer, &MemoryReqs);

	VkMemoryAllocateInfo MemoryAllocInfo{};
	MemoryAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	MemoryAllocInfo.allocationSize = MemoryReqs.size;
	MemoryAllocInfo.memoryTypeIndex = FindMemoryType(MemoryReqs.memoryTypeBits, InProperties);

	if (vkAllocateMemory(GDevice, &MemoryAllocInfo, nullptr, &OutMemory) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate buffer memory.");
	}

	if (vkBindBufferMemory(GDevice, OutBuffer, OutMemory, 0) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to bind buffer memory.");
	}
}

void CopyBuffer(VkBuffer InSrcBuffer, VkBuffer InDstBuffer, VkDeviceSize InSize)
{
	VkCommandBufferAllocateInfo CommandBufferAllocInfo{};
	CommandBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	CommandBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	CommandBufferAllocInfo.commandPool = GCommandPool;
	CommandBufferAllocInfo.commandBufferCount = 1;

	VkCommandBuffer CommandBuffer{};
	vkAllocateCommandBuffers(GDevice, &CommandBufferAllocInfo, &CommandBuffer);

	VkCommandBufferBeginInfo CommandBufferBeginInfo{};
	CommandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	CommandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(CommandBuffer, &CommandBufferBeginInfo);

	VkBufferCopy CopyRegion{};
	CopyRegion.size = InSize;
	vkCmdCopyBuffer(CommandBuffer, InSrcBuffer, InDstBuffer, 1, &CopyRegion);

	vkEndCommandBuffer(CommandBuffer);

	VkSubmitInfo SubmitInfo{};
	SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	SubmitInfo.commandBufferCount = 1;
	SubmitInfo.pCommandBuffers = &CommandBuffer;

	vkQueueSubmit(GGraphicsQueue, 1, &SubmitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(GGraphicsQueue);

	vkFreeCommandBuffers(GDevice, GCommandPool, 1, &CommandBuffer);
}

void RecordCommandBuffer(VkCommandBuffer InCommandBuffer, uint32_t InImageIndex)
{
	VkCommandBufferBeginInfo CommandBufferBeginInfo{};
	CommandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	if (vkBeginCommandBuffer(InCommandBuffer, &CommandBufferBeginInfo) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to begin recording command buffer.");
	}

	VkRenderPassBeginInfo RenderPassBeginInfo{};
	RenderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	RenderPassBeginInfo.renderPass = GRenderPass;
	RenderPassBeginInfo.framebuffer = GSwapchainFramebuffers[InImageIndex];
	RenderPassBeginInfo.renderArea.offset = { 0, 0 };
	RenderPassBeginInfo.renderArea.extent = GSwapchainExtent;

	VkClearValue ClearColor = { { { 0.0f, 0.0f, 0.0f, 1.0f } } };
	RenderPassBeginInfo.clearValueCount = 1;
	RenderPassBeginInfo.pClearValues = &ClearColor;

	vkCmdBeginRenderPass(InCommandBuffer, &RenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(InCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, GPipeline);

	VkViewport Viewport{};
	Viewport.x = 0.0f;
	Viewport.y = 0.0f;
	Viewport.width = (float)GSwapchainExtent.width;
	Viewport.height = (float)GSwapchainExtent.height;
	Viewport.minDepth = 0.0f;
	Viewport.maxDepth = 1.0f;
	vkCmdSetViewport(InCommandBuffer, 0, 1, &Viewport);

	VkRect2D Scissor{};
	Scissor.offset = { 0, 0 };
	Scissor.extent = GSwapchainExtent;
	vkCmdSetScissor(InCommandBuffer, 0, 1, &Scissor);

	VkBuffer VertexBuffers[] = { GVertexBuffer };
	VkDeviceSize Offsets[] = { 0 };
	vkCmdBindVertexBuffers(InCommandBuffer, 0, 1, VertexBuffers, Offsets);

	vkCmdBindIndexBuffer(InCommandBuffer, GIndexBuffer, 0, VK_INDEX_TYPE_UINT16);

	vkCmdDrawIndexed(InCommandBuffer, static_cast<uint32_t>(GIndices.size()), 1, 0, 0, 0);

	vkCmdEndRenderPass(InCommandBuffer);

	if (vkEndCommandBuffer(InCommandBuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to record command buffer.");
	}
};

void InitializeGLFW()
{
	if (glfwInit() == 0)
	{
		throw std::runtime_error("Failed to initialize glfw");
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
}

void CreateWindow()
{
	GWindow = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "VkFirstProgram", nullptr, nullptr);
	if (GWindow == nullptr)
	{
		throw std::runtime_error("Failed to create window");
	}

	glfwSetFramebufferSizeCallback(GWindow, FramebufferResizeCallback);
}

void CreateInstance()
{
	if (GEnableValidationLayers && !SupportsValidationLayer())
	{
		throw std::runtime_error("validation layers requested, but not available.");
	}

	VkApplicationInfo ApplicationInfo{};
	ApplicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	ApplicationInfo.pApplicationName = "VkFirstProgram";
	ApplicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	ApplicationInfo.pEngineName = "No Engine";
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
		InstanceCI.enabledLayerCount = GValidationLayerCount;
		InstanceCI.ppEnabledLayerNames = GValidationLayers;

		VkDebugUtilsMessengerCreateInfoEXT DebugMessengerCI{};
		DebugMessengerCI.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		DebugMessengerCI.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		DebugMessengerCI.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		DebugMessengerCI.pfnUserCallback = DebugMessengerCallback;

		InstanceCI.pNext = &DebugMessengerCI;
	}
	else
	{
		InstanceCI.enabledLayerCount = 0;
		InstanceCI.pNext = nullptr;
	}

	if (vkCreateInstance(&InstanceCI, nullptr, &GInstance) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create instance.");
	}
}

void SetupDebugMessenger()
{
	if (GEnableValidationLayers)
	{
		VkDebugUtilsMessengerCreateInfoEXT DebugMessengerCI{};
		DebugMessengerCI.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		DebugMessengerCI.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		DebugMessengerCI.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
		DebugMessengerCI.pfnUserCallback = DebugMessengerCallback;

		auto Func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(GInstance, "vkCreateDebugUtilsMessengerEXT");
		if (Func != nullptr)
		{
			Func(GInstance, &DebugMessengerCI, nullptr, &GDebugMessenger);
		}
		else
		{
			throw std::runtime_error("Failed to set up debug messenger.");
		}
	}
}

void CreateSurface()
{
	if (glfwCreateWindowSurface(GInstance, GWindow, nullptr, &GSurface) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create window surface.");
	}
}

void PickPhysicalDevice()
{
	uint32_t DeviceCount = 0;
	vkEnumeratePhysicalDevices(GInstance, &DeviceCount, nullptr);

	if (DeviceCount == 0)
	{
		throw std::runtime_error("Failed to find GPUs with Vulkan support.");
	}

	std::vector<VkPhysicalDevice> Devices(DeviceCount);
	vkEnumeratePhysicalDevices(GInstance, &DeviceCount, Devices.data());

	for (VkPhysicalDevice Device : Devices)
	{
		if (IsDeviceSuitable(Device))
		{
			GPhysicalDevice = Device;
			break;
		}
	}

	if (GPhysicalDevice == VK_NULL_HANDLE)
	{
		throw std::runtime_error("Failed to find a suitable GPU");
	}
}

void CreateLogicalDevice()
{
	uint32_t GraphicsFamily = -1;
	uint32_t PresentFamily = -1;

	FindQueueFamilies(GPhysicalDevice, GraphicsFamily, PresentFamily);

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

	VkDeviceCreateInfo DeviceCI{};
	DeviceCI.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	DeviceCI.queueCreateInfoCount = static_cast<uint32_t>(QueueCIs.size());
	DeviceCI.pQueueCreateInfos = QueueCIs.data();

	DeviceCI.pEnabledFeatures = &DeviceFeatures;

	DeviceCI.enabledExtensionCount = GDeviceExtensionCount;
	DeviceCI.ppEnabledExtensionNames = GDeviceExtensions;

	if (GEnableValidationLayers)
	{
		DeviceCI.enabledLayerCount = GValidationLayerCount;
		DeviceCI.ppEnabledLayerNames = GValidationLayers;
	}
	else
	{
		DeviceCI.enabledLayerCount = 0;
	}

	if (vkCreateDevice(GPhysicalDevice, &DeviceCI, nullptr, &GDevice) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create logical device.");
	}

	vkGetDeviceQueue(GDevice, GraphicsFamily, 0, &GGraphicsQueue);
	vkGetDeviceQueue(GDevice, PresentFamily, 0, &GPresentQueue);
}

void CreateSwapchain()
{
	VkSurfaceCapabilitiesKHR Capabilities;
	std::vector<VkSurfaceFormatKHR> Formats;
	std::vector<VkPresentModeKHR> PresentModes;
	QuerySwapchainSupport(GPhysicalDevice, Capabilities, Formats, PresentModes);

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
		glfwGetFramebufferSize(GWindow, &Width, &Height);

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
	SwapchainCI.surface = GSurface;

	SwapchainCI.minImageCount = ChoosenImageCount;
	SwapchainCI.imageFormat = ChoosenFormat.format;
	SwapchainCI.imageColorSpace = ChoosenFormat.colorSpace;
	SwapchainCI.imageExtent = ChoosenSwapchainExtent;
	SwapchainCI.imageArrayLayers = 1;
	SwapchainCI.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	uint32_t GraphicsFamily = -1;
	uint32_t PresentFamily = -1;
	FindQueueFamilies(GPhysicalDevice, GraphicsFamily, PresentFamily);

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

	if (vkCreateSwapchainKHR(GDevice, &SwapchainCI, nullptr, &GSwapchain) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create swap chain.");
	}

	uint32_t SwapchainImageCount;
	vkGetSwapchainImagesKHR(GDevice, GSwapchain, &SwapchainImageCount, nullptr);

	GSwapchainImages.resize(SwapchainImageCount);
	vkGetSwapchainImagesKHR(GDevice, GSwapchain, &SwapchainImageCount, GSwapchainImages.data());

	GSwapchainImageFormat = ChoosenFormat.format;
	GSwapchainExtent = ChoosenSwapchainExtent;
}

void CreateImageViews()
{
	GSwapchainImageViews.resize(GSwapchainImages.size());

	for (uint32_t Idx = 0; Idx < GSwapchainImages.size(); ++Idx)
	{
		VkImageViewCreateInfo ImageViewCI{};
		ImageViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		ImageViewCI.image = GSwapchainImages[Idx];
		ImageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
		ImageViewCI.format = GSwapchainImageFormat;
		ImageViewCI.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		ImageViewCI.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		ImageViewCI.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		ImageViewCI.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		ImageViewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		ImageViewCI.subresourceRange.baseMipLevel = 0;
		ImageViewCI.subresourceRange.levelCount = 1;
		ImageViewCI.subresourceRange.baseArrayLayer = 0;
		ImageViewCI.subresourceRange.layerCount = 1;

		if (vkCreateImageView(GDevice, &ImageViewCI, nullptr, &GSwapchainImageViews[Idx]) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create image views.");
		}
	}
}

void CreateRenderPass()
{
	VkAttachmentDescription ColorAttachmentDesc{};
	ColorAttachmentDesc.format = GSwapchainImageFormat;
	ColorAttachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
	ColorAttachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	ColorAttachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	ColorAttachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	ColorAttachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	ColorAttachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	ColorAttachmentDesc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference ColorAttachmentRef{};
	ColorAttachmentRef.attachment = 0;
	ColorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription SubpassDesc{};
	SubpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	SubpassDesc.colorAttachmentCount = 1;
	SubpassDesc.pColorAttachments = &ColorAttachmentRef;

	VkSubpassDependency SubpassDependency{};
	SubpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	SubpassDependency.dstSubpass = 0;
	SubpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	SubpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	SubpassDependency.srcAccessMask = 0;
	SubpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo RenderPassCI{};
	RenderPassCI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	RenderPassCI.attachmentCount = 1;
	RenderPassCI.pAttachments = &ColorAttachmentDesc;
	RenderPassCI.subpassCount = 1;
	RenderPassCI.pSubpasses = &SubpassDesc;
	RenderPassCI.dependencyCount = 1;
	RenderPassCI.pDependencies = &SubpassDependency;

	if (vkCreateRenderPass(GDevice, &RenderPassCI, nullptr, &GRenderPass) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create render pass.");
	}
}

void CreateGraphicsPipeline()
{
	std::string ShaderDirectory;
	GConfig->Get("ShaderDirectory", ShaderDirectory);

	std::vector<char> VertexShaderCode;
	if (ReadFile(ShaderDirectory + "vert.spv", VertexShaderCode) == false)
	{
		throw std::runtime_error("Failed to open file.");
	}

	std::vector<char> FragmentShaderCode;
	if (ReadFile(ShaderDirectory + "frag.spv", FragmentShaderCode) == false)
	{
		throw std::runtime_error("Failed to open file.");
	}

	VkShaderModule VertexShaderModule = CreateShaderModule(VertexShaderCode);
	VkShaderModule FragmentShaderModule = CreateShaderModule(FragmentShaderCode);

	VkPipelineShaderStageCreateInfo VertexShaderStageCI{};
	VertexShaderStageCI.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	VertexShaderStageCI.stage = VK_SHADER_STAGE_VERTEX_BIT;
	VertexShaderStageCI.module = VertexShaderModule;
	VertexShaderStageCI.pName = "main";

	VkPipelineShaderStageCreateInfo FragmentShaderStageCI{};
	FragmentShaderStageCI.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	FragmentShaderStageCI.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	FragmentShaderStageCI.module = FragmentShaderModule;
	FragmentShaderStageCI.pName = "main";

	VkPipelineShaderStageCreateInfo ShaderStageCIs[] =
	{
		VertexShaderStageCI,
		FragmentShaderStageCI
	};

	VkVertexInputBindingDescription VertexInputBindingDesc{};
	VertexInputBindingDesc.binding = 0;
	VertexInputBindingDesc.stride = sizeof(FVertex);
	VertexInputBindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	VkVertexInputAttributeDescription VertexInputAttributeDescs[2];
	VertexInputAttributeDescs[0].binding = 0;
	VertexInputAttributeDescs[0].location = 0;
	VertexInputAttributeDescs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	VertexInputAttributeDescs[0].offset = offsetof(FVertex, Position);

	VertexInputAttributeDescs[1].binding = 0;
	VertexInputAttributeDescs[1].location = 1;
	VertexInputAttributeDescs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	VertexInputAttributeDescs[1].offset = offsetof(FVertex, Color);

	VkPipelineVertexInputStateCreateInfo VertexInputStateCI{};
	VertexInputStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	VertexInputStateCI.vertexBindingDescriptionCount = 1;
	VertexInputStateCI.pVertexBindingDescriptions = &VertexInputBindingDesc;
	VertexInputStateCI.vertexAttributeDescriptionCount = 2;
	VertexInputStateCI.pVertexAttributeDescriptions = VertexInputAttributeDescs;

	VkPipelineInputAssemblyStateCreateInfo InputAssemblyStateCI{};
	InputAssemblyStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	InputAssemblyStateCI.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	InputAssemblyStateCI.primitiveRestartEnable = VK_FALSE;

	VkPipelineViewportStateCreateInfo ViewportStateCI{};
	ViewportStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	ViewportStateCI.viewportCount = 1;
	ViewportStateCI.scissorCount = 1;

	VkPipelineRasterizationStateCreateInfo RasterizerCI{};
	RasterizerCI.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	RasterizerCI.depthClampEnable = VK_FALSE;
	RasterizerCI.rasterizerDiscardEnable = VK_FALSE;
	RasterizerCI.polygonMode = VK_POLYGON_MODE_FILL;
	RasterizerCI.lineWidth = 1.0f;
	RasterizerCI.cullMode = VK_CULL_MODE_BACK_BIT;
	RasterizerCI.frontFace = VK_FRONT_FACE_CLOCKWISE;
	RasterizerCI.depthBiasEnable = VK_FALSE;

	VkPipelineMultisampleStateCreateInfo MultisampleStateCI{};
	MultisampleStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	MultisampleStateCI.sampleShadingEnable = VK_FALSE;
	MultisampleStateCI.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	VkPipelineColorBlendAttachmentState ColorBlendAttachmentState{};
	ColorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	ColorBlendAttachmentState.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo ColorBlendStateCI{};
	ColorBlendStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	ColorBlendStateCI.logicOpEnable = VK_FALSE;
	ColorBlendStateCI.logicOp = VK_LOGIC_OP_COPY;
	ColorBlendStateCI.attachmentCount = 1;
	ColorBlendStateCI.pAttachments = &ColorBlendAttachmentState;
	ColorBlendStateCI.blendConstants[0] = 0.0f;
	ColorBlendStateCI.blendConstants[1] = 0.0f;
	ColorBlendStateCI.blendConstants[2] = 0.0f;
	ColorBlendStateCI.blendConstants[3] = 0.0f;

	std::vector<VkDynamicState> DynamicStates =
	{
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	VkPipelineDynamicStateCreateInfo DynamicStateCI{};
	DynamicStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	DynamicStateCI.dynamicStateCount = static_cast<uint32_t>(DynamicStates.size());
	DynamicStateCI.pDynamicStates = DynamicStates.data();

	VkPipelineLayoutCreateInfo PipelineLayoutCI{};
	PipelineLayoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	PipelineLayoutCI.setLayoutCount = 0;
	PipelineLayoutCI.pushConstantRangeCount = 0;

	if (vkCreatePipelineLayout(GDevice, &PipelineLayoutCI, nullptr, &GPipelineLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create pipeline layout.");
	}

	VkGraphicsPipelineCreateInfo PipelineCI{};
	PipelineCI.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	PipelineCI.stageCount = 2;
	PipelineCI.pStages = ShaderStageCIs;
	PipelineCI.pVertexInputState = &VertexInputStateCI;
	PipelineCI.pInputAssemblyState = &InputAssemblyStateCI;
	PipelineCI.pViewportState = &ViewportStateCI;
	PipelineCI.pRasterizationState = &RasterizerCI;
	PipelineCI.pMultisampleState = &MultisampleStateCI;
	PipelineCI.pColorBlendState = &ColorBlendStateCI;
	PipelineCI.pDynamicState = &DynamicStateCI;
	PipelineCI.layout = GPipelineLayout;
	PipelineCI.renderPass = GRenderPass;
	PipelineCI.subpass = 0;
	PipelineCI.basePipelineHandle = VK_NULL_HANDLE;

	if (vkCreateGraphicsPipelines(GDevice, VK_NULL_HANDLE, 1, &PipelineCI, nullptr, &GPipeline) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create graphics pipeline.");
	}

	vkDestroyShaderModule(GDevice, VertexShaderModule, nullptr);
	vkDestroyShaderModule(GDevice, FragmentShaderModule, nullptr);
}

void CreateFramebuffers()
{
	GSwapchainFramebuffers.resize(GSwapchainImageViews.size());

	for (size_t Idx = 0; Idx < GSwapchainImageViews.size(); ++Idx)
	{
		VkImageView Attachments[] =
		{
			GSwapchainImageViews[Idx]
		};

		VkFramebufferCreateInfo FramebufferCI{};
		FramebufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		FramebufferCI.renderPass = GRenderPass;
		FramebufferCI.attachmentCount = 1;
		FramebufferCI.pAttachments = Attachments;
		FramebufferCI.width = GSwapchainExtent.width;
		FramebufferCI.height = GSwapchainExtent.height;
		FramebufferCI.layers = 1;

		if (vkCreateFramebuffer(GDevice, &FramebufferCI, nullptr, &GSwapchainFramebuffers[Idx]) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create framebuffer.");
		}
	}
}

void CreateCommandPool()
{
	uint32_t GraphicsFamily = -1;
	uint32_t PresentFamily = -1;
	FindQueueFamilies(GPhysicalDevice, GraphicsFamily, PresentFamily);

	VkCommandPoolCreateInfo CommandPoolCI{};
	CommandPoolCI.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	CommandPoolCI.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	CommandPoolCI.queueFamilyIndex = GraphicsFamily;

	if (vkCreateCommandPool(GDevice, &CommandPoolCI, nullptr, &GCommandPool) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create graphics command pool.");
	}
}

void CreateVertexBuffer()
{
	VkDeviceSize BufferSize = sizeof(FVertex) * GVertices.size();

	VkBuffer StagingBuffer;
	VkDeviceMemory StagingBufferMemory;
	CreateBuffer(
		BufferSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		StagingBuffer,
		StagingBufferMemory);

	void* Data;
	if (vkMapMemory(GDevice, StagingBufferMemory, 0, BufferSize, 0, &Data) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to map buffer memory.");
	}
	memcpy(Data, GVertices.data(), static_cast<size_t>(BufferSize));
	vkUnmapMemory(GDevice, StagingBufferMemory);

	CreateBuffer(
		BufferSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		GVertexBuffer,
		GVertexBufferMemory);

	CopyBuffer(StagingBuffer, GVertexBuffer, BufferSize);

	vkDestroyBuffer(GDevice, StagingBuffer, nullptr);
	vkFreeMemory(GDevice, StagingBufferMemory, nullptr);
}

void CreateIndexBuffer()
{
	VkDeviceSize BufferSize = sizeof(uint16_t) * GIndices.size();

	VkBuffer StagingBuffer;
	VkDeviceMemory StagingBufferMemory;
	CreateBuffer(
		BufferSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		StagingBuffer,
		StagingBufferMemory);

	void* Data;
	if (vkMapMemory(GDevice, StagingBufferMemory, 0, BufferSize, 0, &Data) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to map buffer memory.");
	}
	memcpy(Data, GIndices.data(), static_cast<size_t>(BufferSize));
	vkUnmapMemory(GDevice, StagingBufferMemory);

	CreateBuffer(
		BufferSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		GIndexBuffer,
		GIndexBufferMemory);

	CopyBuffer(StagingBuffer, GIndexBuffer, BufferSize);

	vkDestroyBuffer(GDevice, StagingBuffer, nullptr);
	vkFreeMemory(GDevice, StagingBufferMemory, nullptr);
}

void CreateCommandBuffers()
{
	GCommandBuffers.resize(MAX_CONCURRENT_FRAME);

	VkCommandBufferAllocateInfo CommandBufferAllocInfo{};
	CommandBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	CommandBufferAllocInfo.commandPool = GCommandPool;
	CommandBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	CommandBufferAllocInfo.commandBufferCount = static_cast<uint32_t>(GCommandBuffers.size());

	if (vkAllocateCommandBuffers(GDevice, &CommandBufferAllocInfo, GCommandBuffers.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate command buffers.");
	}
}

void CreateSyncObjects()
{
	GImageAvailableSemaphores.resize(MAX_CONCURRENT_FRAME);
	GRenderFinishedSemaphores.resize(MAX_CONCURRENT_FRAME);
	GFences.resize(MAX_CONCURRENT_FRAME);

	VkSemaphoreCreateInfo SemaphoreCI{};
	SemaphoreCI.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo FenceCI{};
	FenceCI.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	FenceCI.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t Idx = 0; Idx < MAX_CONCURRENT_FRAME; ++Idx)
	{
		if (vkCreateSemaphore(GDevice, &SemaphoreCI, nullptr, &GImageAvailableSemaphores[Idx]) != VK_SUCCESS ||
			vkCreateSemaphore(GDevice, &SemaphoreCI, nullptr, &GRenderFinishedSemaphores[Idx]) != VK_SUCCESS ||
			vkCreateFence(GDevice, &FenceCI, nullptr, &GFences[Idx]) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create synchronization objects for a frame.");
		}
	}
}

void WaitIdle()
{
	vkDeviceWaitIdle(GDevice);
}

void RecreateSwapchain()
{
	int Width = 0, Height = 0;
	glfwGetFramebufferSize(GWindow, &Width, &Height);

	while (Width == 0 || Height == 0)
	{
		glfwGetFramebufferSize(GWindow, &Width, &Height);
		glfwWaitEvents();
	}

	vkDeviceWaitIdle(GDevice);

	CleanupSwapchain();

	CreateSwapchain();
	CreateImageViews();
	CreateFramebuffers();
}

void CleanupSwapchain()
{
	for (VkFramebuffer Framebuffer : GSwapchainFramebuffers)
	{
		vkDestroyFramebuffer(GDevice, Framebuffer, nullptr);
	}

	for (VkImageView ImageView : GSwapchainImageViews)
	{
		vkDestroyImageView(GDevice, ImageView, nullptr);
	}

	vkDestroySwapchainKHR(GDevice, GSwapchain, nullptr);
}

void Render()
{
	vkWaitForFences(GDevice, 1, &GFences[GCurrentFrame], VK_TRUE, UINT64_MAX);

	uint32_t ImageIndex;
	VkResult AcquireResult = vkAcquireNextImageKHR(
		GDevice,
		GSwapchain,
		UINT64_MAX,
		GImageAvailableSemaphores[GCurrentFrame],
		VK_NULL_HANDLE,
		&ImageIndex);

	if (AcquireResult == VK_ERROR_OUT_OF_DATE_KHR)
	{
		RecreateSwapchain();
		return;
	}
	else if (AcquireResult != VK_SUCCESS && AcquireResult != VK_SUBOPTIMAL_KHR)
	{
		throw std::runtime_error("Failed to acquire swap chain image.");
	}

	vkResetFences(GDevice, 1, &GFences[GCurrentFrame]);

	vkResetCommandBuffer(GCommandBuffers[GCurrentFrame], 0);
	RecordCommandBuffer(GCommandBuffers[GCurrentFrame], ImageIndex);

	VkSemaphore WaitSemaphores[] = { GImageAvailableSemaphores[GCurrentFrame] };
	VkPipelineStageFlags WaitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	VkSemaphore SignalSemaphores[] = { GRenderFinishedSemaphores[GCurrentFrame] };

	VkSubmitInfo SubmitInfo{};
	SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	SubmitInfo.waitSemaphoreCount = 1;
	SubmitInfo.pWaitSemaphores = WaitSemaphores;
	SubmitInfo.pWaitDstStageMask = WaitStages;
	SubmitInfo.commandBufferCount = 1;
	SubmitInfo.pCommandBuffers = &GCommandBuffers[GCurrentFrame];
	SubmitInfo.signalSemaphoreCount = 1;
	SubmitInfo.pSignalSemaphores = SignalSemaphores;

	if (vkQueueSubmit(GGraphicsQueue, 1, &SubmitInfo, GFences[GCurrentFrame]) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to submit draw command buffer.");
	}

	VkSwapchainKHR Swapchains[] = { GSwapchain };

	VkPresentInfoKHR PresentInfo{};
	PresentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	PresentInfo.waitSemaphoreCount = 1;
	PresentInfo.pWaitSemaphores = SignalSemaphores;
	PresentInfo.swapchainCount = 1;
	PresentInfo.pSwapchains = Swapchains;
	PresentInfo.pImageIndices = &ImageIndex;

	VkResult PresentResult = vkQueuePresentKHR(GPresentQueue, &PresentInfo);
	if (PresentResult == VK_ERROR_OUT_OF_DATE_KHR || PresentResult == VK_SUBOPTIMAL_KHR || GFramebufferResized)
	{
		GFramebufferResized = false;
		RecreateSwapchain();
	}
	else if (PresentResult != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to present swap chain image.");
	}

	GCurrentFrame = (GCurrentFrame + 1) % MAX_CONCURRENT_FRAME;
}

void Cleanup()
{
	CleanupSwapchain();

	vkDestroyPipelineLayout(GDevice, GPipelineLayout, nullptr);
	vkDestroyPipeline(GDevice, GPipeline, nullptr);

	vkDestroyRenderPass(GDevice, GRenderPass, nullptr);

	vkDestroyBuffer(GDevice, GVertexBuffer, nullptr);
	vkFreeMemory(GDevice, GVertexBufferMemory, nullptr);

	vkDestroyBuffer(GDevice, GIndexBuffer, nullptr);
	vkFreeMemory(GDevice, GIndexBufferMemory, nullptr);

	for (size_t Idx = 0; Idx < MAX_CONCURRENT_FRAME; ++Idx)
	{
		vkDestroySemaphore(GDevice, GImageAvailableSemaphores[Idx], nullptr);
		vkDestroySemaphore(GDevice, GRenderFinishedSemaphores[Idx], nullptr);
		vkDestroyFence(GDevice, GFences[Idx], nullptr);
	}

	vkDestroyCommandPool(GDevice, GCommandPool, nullptr);

	vkDestroyDevice(GDevice, nullptr);

	if (GEnableValidationLayers)
	{
		auto Func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(GInstance, "vkDestroyDebugUtilsMessengerEXT");
		if (Func != nullptr)
		{
			Func(GInstance, GDebugMessenger, nullptr);
		}
	}

	vkDestroySurfaceKHR(GInstance, GSurface, nullptr);
	vkDestroyInstance(GInstance, nullptr);

	glfwDestroyWindow(GWindow);
	glfwTerminate();
}
