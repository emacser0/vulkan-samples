#include "Rendering.h"
#include "VulkanHelpers.h"
#include "Utils.h"
#include "Engine.h"
#include "Config.h"
#include "Camera.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "glm/gtc/matrix_transform.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/quaternion.hpp"

static std::vector<const char*> GValidationLayers  =
{
	"VK_LAYER_KHRONOS_validation"
};

static std::vector<const char*> GDeviceExtensions =
{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

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

VkDescriptorPool GDescriptorPool;
VkDescriptorSetLayout GDescriptorSetLayout;
std::vector<VkDescriptorSet> GDescriptorSets;

VkImage GTextureImage;
VkDeviceMemory GTextureImageMemory;
VkImageView GTextureImageView;
VkSampler GTextureSampler;

VkBuffer GVertexBuffer;
VkDeviceMemory GVertexBufferMemory;
VkBuffer GIndexBuffer;
VkDeviceMemory GIndexBufferMemory;

std::vector<FUniformBuffer> GUniformBuffers;

std::vector<VkCommandBuffer> GCommandBuffers;

std::vector<VkSemaphore> GImageAvailableSemaphores;
std::vector<VkSemaphore> GRenderFinishedSemaphores;
std::vector<VkFence> GFences;

bool GFramebufferResized = false;

uint32_t GCurrentFrame = 0;

const std::vector<FVertex> GVertices =
{
	{{ -0.5f, -0.5f, -0.5f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f }},
	{{ 0.5f, -0.5f, -0.5f }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f }},
	{{ 0.5f,  0.5f, -0.5f }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f }},
	{{ -0.5f,  0.5f, -0.5f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f }},
	{{ -0.5f, -0.5f,  0.5f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f }},
	{{ 0.5f, -0.5f,  0.5f }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f }},
	{{ 0.5f,  0.5f,  0.5f }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f }},
	{{ -0.5f,  0.5f,  0.5f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f }}
};

const std::vector<uint32_t> GIndices =
{
	0, 1, 3, 3, 1, 2,
	1, 5, 2, 2, 5, 6,
	5, 4, 6, 6, 4, 7,
	4, 0, 7, 7, 0, 3,
	3, 2, 7, 7, 2, 6,
	4, 5, 0, 0, 5, 1
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

	vkCmdBindIndexBuffer(InCommandBuffer, GIndexBuffer, 0, VK_INDEX_TYPE_UINT32);

	vkCmdBindDescriptorSets(InCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, GPipelineLayout, 0, 1, &GDescriptorSets[GCurrentFrame], 0, nullptr);

	vkCmdDrawIndexed(InCommandBuffer, static_cast<uint32_t>(GIndices.size()), 1, 0, 0, 0);

	vkCmdEndRenderPass(InCommandBuffer);

	if (vkEndCommandBuffer(InCommandBuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to record command buffer.");
	}
}

void InitializeGLFW()
{
	if (glfwInit() == 0)
	{
		throw std::runtime_error("Failed to initialize glfw");
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
}

void CreateGLFWWindow()
{
	int32_t WindowWidth;
	int32_t WindowHeight;
	GConfig->Get("WindowWidth", WindowWidth);
	GConfig->Get("WindowHeight", WindowHeight);
	GWindow = glfwCreateWindow(WindowWidth, WindowHeight, "VkCube", nullptr, nullptr);
	if (GWindow == nullptr)
	{
		throw std::runtime_error("Failed to create window");
	}

	glfwSetFramebufferSizeCallback(GWindow, FramebufferResizeCallback);
	glfwSetInputMode(GWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void CreateInstance()
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
		if (Vk::IsDeviceSuitable(Device, GSurface, GDeviceExtensions))
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

	Vk::FindQueueFamilies(GPhysicalDevice, GSurface, GraphicsFamily, PresentFamily);

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

	VkDeviceCreateInfo DeviceCI{};
	DeviceCI.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	DeviceCI.queueCreateInfoCount = static_cast<uint32_t>(QueueCIs.size());
	DeviceCI.pQueueCreateInfos = QueueCIs.data();

	DeviceCI.pEnabledFeatures = &DeviceFeatures;

	DeviceCI.enabledExtensionCount = static_cast<uint32_t>(GDeviceExtensions.size());
	DeviceCI.ppEnabledExtensionNames = GDeviceExtensions.data();

	if (GEnableValidationLayers)
	{
		DeviceCI.enabledLayerCount = static_cast<uint32_t>(GValidationLayers.size());
		DeviceCI.ppEnabledLayerNames = GValidationLayers.data();
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
	Vk::QuerySwapchainSupport(GPhysicalDevice, GSurface, Capabilities, Formats, PresentModes);

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
	Vk::FindQueueFamilies(GPhysicalDevice, GSurface, GraphicsFamily, PresentFamily);

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

void CreateDescriptorSetLayout()
{
	VkDescriptorSetLayoutBinding UBOLayoutBinding{};
	UBOLayoutBinding.binding = 0;
	UBOLayoutBinding.descriptorCount = 1;
	UBOLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	UBOLayoutBinding.pImmutableSamplers = nullptr;
	UBOLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkDescriptorSetLayoutBinding SamplerLayoutBinding{};
	SamplerLayoutBinding.binding = 1;
	SamplerLayoutBinding.descriptorCount = 1;
	SamplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	SamplerLayoutBinding.pImmutableSamplers = nullptr;
	SamplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	std::vector<VkDescriptorSetLayoutBinding> Bindings =
	{
		UBOLayoutBinding,
		SamplerLayoutBinding
	};

	VkDescriptorSetLayoutCreateInfo DescriptorSetLayoutCI{};
	DescriptorSetLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	DescriptorSetLayoutCI.bindingCount = static_cast<uint32_t>(Bindings.size());
	DescriptorSetLayoutCI.pBindings = Bindings.data();

	if (vkCreateDescriptorSetLayout(GDevice, &DescriptorSetLayoutCI, nullptr, &GDescriptorSetLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create descriptor set layout.");
	}
}

void CreateGraphicsPipeline()
{
	std::string ShaderDirectory;
	GConfig->Get("ShaderDirectory", ShaderDirectory);

	std::vector<char> VertexShaderCode;
	if (ReadFile(ShaderDirectory + "main.vert.spv", VertexShaderCode) == false)
	{
		throw std::runtime_error("Failed to open file.");
	}

	std::vector<char> FragmentShaderCode;
	if (ReadFile(ShaderDirectory + "main.frag.spv", FragmentShaderCode) == false)
	{
		throw std::runtime_error("Failed to open file.");
	}

	VkShaderModule VertexShaderModule = Vk::CreateShaderModule(GDevice, VertexShaderCode);
	VkShaderModule FragmentShaderModule = Vk::CreateShaderModule(GDevice, FragmentShaderCode);

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

	VkVertexInputAttributeDescription VertexInputAttributeDescs[3];
	VertexInputAttributeDescs[0].binding = 0;
	VertexInputAttributeDescs[0].location = 0;
	VertexInputAttributeDescs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	VertexInputAttributeDescs[0].offset = offsetof(FVertex, Position);

	VertexInputAttributeDescs[1].binding = 0;
	VertexInputAttributeDescs[1].location = 1;
	VertexInputAttributeDescs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	VertexInputAttributeDescs[1].offset = offsetof(FVertex, Color);

	VertexInputAttributeDescs[2].binding = 0;
	VertexInputAttributeDescs[2].location = 2;
	VertexInputAttributeDescs[2].format = VK_FORMAT_R32G32_SFLOAT;
	VertexInputAttributeDescs[2].offset = offsetof(FVertex, TexCoords);

	VkPipelineVertexInputStateCreateInfo VertexInputStateCI{};
	VertexInputStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	VertexInputStateCI.vertexBindingDescriptionCount = 1U;
	VertexInputStateCI.pVertexBindingDescriptions = &VertexInputBindingDesc;
	VertexInputStateCI.vertexAttributeDescriptionCount = 3U;
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
	RasterizerCI.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
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
	PipelineLayoutCI.setLayoutCount = 1;
	PipelineLayoutCI.pSetLayouts = &GDescriptorSetLayout;

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
	Vk::FindQueueFamilies(GPhysicalDevice, GSurface, GraphicsFamily, PresentFamily);

	VkCommandPoolCreateInfo CommandPoolCI{};
	CommandPoolCI.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	CommandPoolCI.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	CommandPoolCI.queueFamilyIndex = GraphicsFamily;

	if (vkCreateCommandPool(GDevice, &CommandPoolCI, nullptr, &GCommandPool) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create graphics command pool.");
	}
}

void CreateTextureImage()
{
	std::string ImageDirectory;
	GConfig->Get("ImageDirectory", ImageDirectory);

	std::string TextureFilename = ImageDirectory + "texture.jpg";

	int TextureWidth, TextureHeight, TextureChannels;
	stbi_uc* Pixels = stbi_load(TextureFilename.c_str(), &TextureWidth, &TextureHeight, &TextureChannels, STBI_rgb_alpha);

	VkDeviceSize ImageSize = TextureWidth * TextureHeight * 4;
	if (Pixels == nullptr)
	{
		throw std::runtime_error("Failed to load texture image.");
	}

	VkBuffer StagingBuffer;
	VkDeviceMemory StagingBufferMemory;
	Vk::CreateBuffer(
		GPhysicalDevice,
		GDevice,
		ImageSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		StagingBuffer,
		StagingBufferMemory);

	void* Data;
	if (vkMapMemory(GDevice, StagingBufferMemory, 0, ImageSize, 0, &Data) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to map buffer memory.");
	}
	memcpy(Data, Pixels, static_cast<size_t>(ImageSize));
	vkUnmapMemory(GDevice, StagingBufferMemory);

	Vk::CreateImage(
		GPhysicalDevice,
		GDevice,
		TextureWidth,
		TextureHeight,
		VK_FORMAT_R8G8B8A8_SRGB,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		GTextureImage,
		GTextureImageMemory);

	Vk::TransitionImageLayout(
		GDevice,
		GCommandPool,
		GGraphicsQueue,
		GTextureImage,
		VK_FORMAT_R8G8B8A8_SRGB,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	Vk::CopyBufferToImage(
		GDevice,
		GCommandPool,
		GGraphicsQueue,
		StagingBuffer,
		GTextureImage,
		static_cast<uint32_t>(TextureWidth),
		static_cast<uint32_t>(TextureHeight));
	Vk::TransitionImageLayout(
		GDevice,
		GCommandPool,
		GGraphicsQueue,
		GTextureImage,
		VK_FORMAT_R8G8B8A8_SRGB,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	vkDestroyBuffer(GDevice, StagingBuffer, nullptr);
	vkFreeMemory(GDevice, StagingBufferMemory, nullptr);
}

void CreateTextureImageView()
{
	GTextureImageView = Vk::CreateImageView(GDevice, GTextureImage, VK_FORMAT_R8G8B8A8_SRGB);
}

void CreateTextureSampler()
{
	VkPhysicalDeviceProperties Properties{};
	vkGetPhysicalDeviceProperties(GPhysicalDevice, &Properties);

	VkSamplerCreateInfo SamplerCI{};
	SamplerCI.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	SamplerCI.magFilter = VK_FILTER_LINEAR;
	SamplerCI.minFilter = VK_FILTER_LINEAR;
	SamplerCI.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	SamplerCI.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	SamplerCI.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	SamplerCI.anisotropyEnable = VK_TRUE;
	SamplerCI.maxAnisotropy = Properties.limits.maxSamplerAnisotropy;
	SamplerCI.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	SamplerCI.unnormalizedCoordinates = VK_FALSE;
	SamplerCI.compareEnable = VK_FALSE;
	SamplerCI.compareOp = VK_COMPARE_OP_ALWAYS;
	SamplerCI.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

	if (vkCreateSampler(GDevice, &SamplerCI, nullptr, &GTextureSampler) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create texture sampler!");
	}
}

void CreateVertexBuffer()
{
	VkDeviceSize BufferSize = sizeof(FVertex) * GVertices.size();

	VkBuffer StagingBuffer;
	VkDeviceMemory StagingBufferMemory;
	Vk::CreateBuffer(
		GPhysicalDevice,
		GDevice,
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

	Vk::CreateBuffer(
		GPhysicalDevice,
		GDevice,
		BufferSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		GVertexBuffer,
		GVertexBufferMemory);

	Vk::CopyBuffer(GDevice, GCommandPool, GGraphicsQueue, StagingBuffer, GVertexBuffer, BufferSize);

	vkDestroyBuffer(GDevice, StagingBuffer, nullptr);
	vkFreeMemory(GDevice, StagingBufferMemory, nullptr);
}

void CreateIndexBuffer()
{
	VkDeviceSize BufferSize = sizeof(uint32_t) * GIndices.size();

	VkBuffer StagingBuffer;
	VkDeviceMemory StagingBufferMemory;
	Vk::CreateBuffer(
		GPhysicalDevice,
		GDevice,
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

	Vk::CreateBuffer(
		GPhysicalDevice,
		GDevice,
		BufferSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		GIndexBuffer,
		GIndexBufferMemory);

	Vk::CopyBuffer(GDevice, GCommandPool, GGraphicsQueue, StagingBuffer, GIndexBuffer, BufferSize);

	vkDestroyBuffer(GDevice, StagingBuffer, nullptr);
	vkFreeMemory(GDevice, StagingBufferMemory, nullptr);
}

void CreateUniformBuffers()
{
	VkDeviceSize UniformBufferSize = sizeof(FUniformBufferObject);

	GUniformBuffers.resize(MAX_CONCURRENT_FRAME);

	for (size_t Idx = 0; Idx < MAX_CONCURRENT_FRAME; ++Idx)
	{
		Vk::CreateBuffer(
			GPhysicalDevice,
			GDevice,
			UniformBufferSize,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			GUniformBuffers[Idx].Buffer,
			GUniformBuffers[Idx].Memory);

		vkMapMemory(GDevice, GUniformBuffers[Idx].Memory, 0, UniformBufferSize, 0, &GUniformBuffers[Idx].Mapped);
	}
}

void CreateDescriptorPool()
{
	VkDescriptorPoolSize PoolSizes[2];
	PoolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	PoolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_CONCURRENT_FRAME);
	PoolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	PoolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_CONCURRENT_FRAME);

	VkDescriptorPoolCreateInfo DescriptorPoolCI{};
	DescriptorPoolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	DescriptorPoolCI.poolSizeCount = 2;
	DescriptorPoolCI.pPoolSizes = PoolSizes;
	DescriptorPoolCI.maxSets = static_cast<uint32_t>(MAX_CONCURRENT_FRAME);

	if (vkCreateDescriptorPool(GDevice, &DescriptorPoolCI, nullptr, &GDescriptorPool) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create descriptor pool.");
	}
}

void CreateDescriptorSets()
{
	std::vector<VkDescriptorSetLayout> Layouts(MAX_CONCURRENT_FRAME, GDescriptorSetLayout);
	VkDescriptorSetAllocateInfo DescriptorSetAllocInfo{};
	DescriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	DescriptorSetAllocInfo.descriptorPool = GDescriptorPool;
	DescriptorSetAllocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_CONCURRENT_FRAME);
	DescriptorSetAllocInfo.pSetLayouts = Layouts.data();

	GDescriptorSets.resize(MAX_CONCURRENT_FRAME);
	if (vkAllocateDescriptorSets(GDevice, &DescriptorSetAllocInfo, GDescriptorSets.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate Descriptor sets!");
	}

	for (size_t Idx = 0; Idx < MAX_CONCURRENT_FRAME; Idx++)
	{
		VkDescriptorBufferInfo BufferInfo{};
		BufferInfo.buffer = GUniformBuffers[Idx].Buffer;
		BufferInfo.offset = 0;
		BufferInfo.range = sizeof(FUniformBufferObject);

		VkDescriptorImageInfo ImageInfo{};
		ImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		ImageInfo.imageView = GTextureImageView;
		ImageInfo.sampler = GTextureSampler;

		std::array<VkWriteDescriptorSet, 2> DescriptorWrites{};

		DescriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		DescriptorWrites[0].dstSet = GDescriptorSets[Idx];
		DescriptorWrites[0].dstBinding = 0;
		DescriptorWrites[0].dstArrayElement = 0;
		DescriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		DescriptorWrites[0].descriptorCount = 1;
		DescriptorWrites[0].pBufferInfo = &BufferInfo;

		DescriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		DescriptorWrites[1].dstSet = GDescriptorSets[Idx];
		DescriptorWrites[1].dstBinding = 1;
		DescriptorWrites[1].dstArrayElement = 0;
		DescriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		DescriptorWrites[1].descriptorCount = 1;
		DescriptorWrites[1].pImageInfo = &ImageInfo;

		vkUpdateDescriptorSets(GDevice, static_cast<uint32_t>(DescriptorWrites.size()), DescriptorWrites.data(), 0, nullptr);
	}
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

void UpdateUniformBuffer()
{
	FCamera* Camera = GEngine->GetCamera();
	assert(Camera != nullptr);

	float FOVRadians = glm::radians(Camera->GetFOV());
	float AspectRatio = GSwapchainExtent.width / (float)GSwapchainExtent.height;

	FUniformBufferObject UBO{};
	UBO.Model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -2.0f));
	UBO.View = Camera->GetViewMatrix();
	UBO.Projection = glm::perspectiveRH(FOVRadians, AspectRatio, 0.1f, 10.0f);

	memcpy(GUniformBuffers[GCurrentFrame].Mapped, &UBO, sizeof(FUniformBufferObject));
}

void Render(float InDeltaTime)
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

	UpdateUniformBuffer();

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

	vkDestroyDescriptorPool(GDevice, GDescriptorPool, nullptr);
	vkDestroyDescriptorSetLayout(GDevice, GDescriptorSetLayout, nullptr);

	vkDestroyImage(GDevice, GTextureImage, nullptr);
	vkFreeMemory(GDevice, GTextureImageMemory, nullptr);
	vkDestroyImageView(GDevice, GTextureImageView, nullptr);
	vkDestroySampler(GDevice, GTextureSampler, nullptr);

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