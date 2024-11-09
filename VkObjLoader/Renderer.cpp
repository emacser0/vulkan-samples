#include "Renderer.h"
#include "VulkanHelpers.h"
#include "Utils.h"
#include "MeshUtils.h"
#include "Engine.h"
#include "Config.h"
#include "Camera.h"

#include "glm/gtc/matrix_transform.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/quaternion.hpp"

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

static bool bFramebufferResized = false;

void FramebufferResizeCallback(GLFWwindow* InWindow, int InWidth, int InHeight)
{
	bFramebufferResized = true;
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

FSingleObjectRenderer::FSingleObjectRenderer(GLFWwindow* InWindow)
	: FRenderer(InWindow)
{
	assert(Window != nullptr);

	glfwSetFramebufferSizeCallback(Window, FramebufferResizeCallback);

	CreateInstance();
	SetupDebugMessenger();
	CreateSurface();
	PickPhysicalDevice();
	CreateLogicalDevice();
}

FSingleObjectRenderer::~FSingleObjectRenderer()
{
	CleanupSwapchain();

	vkDestroyPipelineLayout(Device, PipelineLayout, nullptr);
	vkDestroyPipeline(Device, Pipeline, nullptr);
	vkDestroyRenderPass(Device, RenderPass, nullptr);

	vkDestroyDescriptorPool(Device, DescriptorPool, nullptr);
	vkDestroyDescriptorSetLayout(Device, DescriptorSetLayout, nullptr);

	vkDestroyImage(Device, TextureImage, nullptr);
	vkFreeMemory(Device, TextureImageMemory, nullptr);
	vkDestroyImageView(Device, TextureImageView, nullptr);
	vkDestroySampler(Device, TextureSampler, nullptr);

	vkDestroyBuffer(Device, VertexBuffer, nullptr);
	vkFreeMemory(Device, VertexBufferMemory, nullptr);

	vkDestroyBuffer(Device, IndexBuffer, nullptr);
	vkFreeMemory(Device, IndexBufferMemory, nullptr);

	for (size_t Idx = 0; Idx < MAX_CONCURRENT_FRAME; ++Idx)
	{
		vkDestroySemaphore(Device, ImageAvailableSemaphores[Idx], nullptr);
		vkDestroySemaphore(Device, RenderFinishedSemaphores[Idx], nullptr);
		vkDestroyFence(Device, Fences[Idx], nullptr);
	}

	vkDestroyCommandPool(Device, CommandPool, nullptr);

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

void FSingleObjectRenderer::Ready()
{	
	CreateSwapchain();
	CreateImageViews();
	CreateRenderPass();
	CreateDescriptorSetLayout();
	CreateGraphicsPipeline();
	CreateFramebuffers();
	CreateCommandPool();
	CreateTextureImage();
	CreateTextureImageView();
	CreateTextureSampler();
	CreateVertexBuffer();
	CreateIndexBuffer();
	CreateUniformBuffers();
	CreateDescriptorPool();
	CreateDescriptorSets();
	CreateCommandBuffers();
	CreateSyncObjects();
}

void FSingleObjectRenderer::RecordCommandBuffer(VkCommandBuffer InCommandBuffer, uint32_t InImageIndex)
{
	VkCommandBufferBeginInfo CommandBufferBeginInfo{};
	CommandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	if (vkBeginCommandBuffer(InCommandBuffer, &CommandBufferBeginInfo) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to begin recording command buffer.");
	}

	VkRenderPassBeginInfo RenderPassBeginInfo{};
	RenderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	RenderPassBeginInfo.renderPass = RenderPass;
	RenderPassBeginInfo.framebuffer = SwapchainFramebuffers[InImageIndex];
	RenderPassBeginInfo.renderArea.offset = { 0, 0 };
	RenderPassBeginInfo.renderArea.extent = SwapchainExtent;

	VkClearValue ClearColor = { { { 0.0f, 0.0f, 0.0f, 1.0f } } };
	RenderPassBeginInfo.clearValueCount = 1;
	RenderPassBeginInfo.pClearValues = &ClearColor;

	vkCmdBeginRenderPass(InCommandBuffer, &RenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(InCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipeline);

	VkViewport Viewport{};
	Viewport.x = 0.0f;
	Viewport.y = 0.0f;
	Viewport.width = (float)SwapchainExtent.width;
	Viewport.height = (float)SwapchainExtent.height;
	Viewport.minDepth = 0.0f;
	Viewport.maxDepth = 1.0f;
	vkCmdSetViewport(InCommandBuffer, 0, 1, &Viewport);

	VkRect2D Scissor{};
	Scissor.offset = { 0, 0 };
	Scissor.extent = SwapchainExtent;
	vkCmdSetScissor(InCommandBuffer, 0, 1, &Scissor);

	VkBuffer VertexBuffers[] = { VertexBuffer };
	VkDeviceSize Offsets[] = { 0 };
	vkCmdBindVertexBuffers(InCommandBuffer, 0, 1, VertexBuffers, Offsets);

	vkCmdBindIndexBuffer(InCommandBuffer, IndexBuffer, 0, VK_INDEX_TYPE_UINT16);

	vkCmdBindDescriptorSets(InCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, PipelineLayout, 0, 1, &DescriptorSets[GCurrentFrame], 0, nullptr);

	vkCmdDrawIndexed(InCommandBuffer, static_cast<uint32_t>(Indices.size()), 1, 0, 0, 0);

	vkCmdEndRenderPass(InCommandBuffer);

	if (vkEndCommandBuffer(InCommandBuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to record command buffer.");
	}
}

void FSingleObjectRenderer::CreateInstance()
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

void FSingleObjectRenderer::SetupDebugMessenger()
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

void FSingleObjectRenderer::CreateSurface()
{
	if (glfwCreateWindowSurface(Instance, Window, nullptr, &Surface) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create window surface.");
	}
}

void FSingleObjectRenderer::PickPhysicalDevice()
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

void FSingleObjectRenderer::CreateLogicalDevice()
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

	vkGetDeviceQueue(Device, GraphicsFamily, 0, &GraphicsQueue);
	vkGetDeviceQueue(Device, PresentFamily, 0, &PresentQueue);
}

void FSingleObjectRenderer::CreateSwapchain()
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

void FSingleObjectRenderer::CreateImageViews()
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

void FSingleObjectRenderer::CreateRenderPass()
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

	if (vkCreateRenderPass(Device, &RenderPassCI, nullptr, &RenderPass) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create render pass.");
	}
}

void FSingleObjectRenderer::CreateDescriptorSetLayout()
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

	if (vkCreateDescriptorSetLayout(Device, &DescriptorSetLayoutCI, nullptr, &DescriptorSetLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create descriptor set layout.");
	}
}

void FSingleObjectRenderer::CreateGraphicsPipeline()
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

	VkShaderModule VertexShaderModule = Vk::CreateShaderModule(Device, VertexShaderCode);
	VkShaderModule FragmentShaderModule = Vk::CreateShaderModule(Device, FragmentShaderCode);

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
	PipelineLayoutCI.setLayoutCount = 1;
	PipelineLayoutCI.pSetLayouts = &DescriptorSetLayout;

	if (vkCreatePipelineLayout(Device, &PipelineLayoutCI, nullptr, &PipelineLayout) != VK_SUCCESS)
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
	PipelineCI.layout = PipelineLayout;
	PipelineCI.renderPass = RenderPass;
	PipelineCI.subpass = 0;
	PipelineCI.basePipelineHandle = VK_NULL_HANDLE;

	if (vkCreateGraphicsPipelines(Device, VK_NULL_HANDLE, 1, &PipelineCI, nullptr, &Pipeline) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create graphics pipeline.");
	}

	vkDestroyShaderModule(Device, VertexShaderModule, nullptr);
	vkDestroyShaderModule(Device, FragmentShaderModule, nullptr);
}

void FSingleObjectRenderer::CreateFramebuffers()
{
	SwapchainFramebuffers.resize(SwapchainImageViews.size());

	for (size_t Idx = 0; Idx < SwapchainImageViews.size(); ++Idx)
	{
		VkImageView Attachments[] =
		{
			SwapchainImageViews[Idx]
		};

		VkFramebufferCreateInfo FramebufferCI{};
		FramebufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		FramebufferCI.renderPass = RenderPass;
		FramebufferCI.attachmentCount = 1;
		FramebufferCI.pAttachments = Attachments;
		FramebufferCI.width = SwapchainExtent.width;
		FramebufferCI.height = SwapchainExtent.height;
		FramebufferCI.layers = 1;

		if (vkCreateFramebuffer(Device, &FramebufferCI, nullptr, &SwapchainFramebuffers[Idx]) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create framebuffer.");
		}
	}
}

void FSingleObjectRenderer::CreateCommandPool()
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

void FSingleObjectRenderer::CreateTextureImage()
{
	VkDeviceSize ImageSize = Texture.Width * Texture.Height * 4U;

	VkBuffer StagingBuffer;
	VkDeviceMemory StagingBufferMemory;
	Vk::CreateBuffer(
		PhysicalDevice,
		Device,
		ImageSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		StagingBuffer,
		StagingBufferMemory);

	void* Data;
	if (vkMapMemory(Device, StagingBufferMemory, 0, ImageSize, 0, &Data) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to map buffer memory.");
	}
	memcpy(Data, Texture.Pixels, static_cast<size_t>(ImageSize));
	vkUnmapMemory(Device, StagingBufferMemory);

	Vk::CreateImage(
		PhysicalDevice,
		Device,
		static_cast<uint32_t>(Texture.Width),
		static_cast<uint32_t>(Texture.Height),
		VK_FORMAT_R8G8B8A8_SRGB,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		TextureImage,
		TextureImageMemory);

	Vk::TransitionImageLayout(
		Device,
		CommandPool,
		GraphicsQueue,
		TextureImage,
		VK_FORMAT_R8G8B8A8_SRGB,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	Vk::CopyBufferToImage(
		Device,
		CommandPool,
		GraphicsQueue,
		StagingBuffer,
		TextureImage,
		static_cast<uint32_t>(Texture.Width),
		static_cast<uint32_t>(Texture.Height));
	Vk::TransitionImageLayout(
		Device,
		CommandPool,
		GraphicsQueue,
		TextureImage,
		VK_FORMAT_R8G8B8A8_SRGB,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	vkDestroyBuffer(Device, StagingBuffer, nullptr);
	vkFreeMemory(Device, StagingBufferMemory, nullptr);
}

void FSingleObjectRenderer::CreateTextureImageView()
{
	TextureImageView = Vk::CreateImageView(Device, TextureImage, VK_FORMAT_R8G8B8A8_SRGB);
}

void FSingleObjectRenderer::CreateTextureSampler()
{
	VkPhysicalDeviceProperties Properties{};
	vkGetPhysicalDeviceProperties(PhysicalDevice, &Properties);

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

	if (vkCreateSampler(Device, &SamplerCI, nullptr, &TextureSampler) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create texture sampler!");
	}
}

void FSingleObjectRenderer::CreateVertexBuffer()
{
	VkDeviceSize BufferSize = sizeof(FVertex) * Vertices.size();

	VkBuffer StagingBuffer;
	VkDeviceMemory StagingBufferMemory;
	Vk::CreateBuffer(
		PhysicalDevice,
		Device,
		BufferSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		StagingBuffer,
		StagingBufferMemory);

	void* Data;
	if (vkMapMemory(Device, StagingBufferMemory, 0, BufferSize, 0, &Data) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to map buffer memory.");
	}
	memcpy(Data, Vertices.data(), static_cast<size_t>(BufferSize));
	vkUnmapMemory(Device, StagingBufferMemory);

	Vk::CreateBuffer(
		PhysicalDevice,
		Device,
		BufferSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		VertexBuffer,
		VertexBufferMemory);

	Vk::CopyBuffer(Device, CommandPool, GraphicsQueue, StagingBuffer, VertexBuffer, BufferSize);

	vkDestroyBuffer(Device, StagingBuffer, nullptr);
	vkFreeMemory(Device, StagingBufferMemory, nullptr);
}

void FSingleObjectRenderer::CreateIndexBuffer()
{
	VkDeviceSize BufferSize = sizeof(uint16_t) * Indices.size();

	VkBuffer StagingBuffer;
	VkDeviceMemory StagingBufferMemory;
	Vk::CreateBuffer(
		PhysicalDevice,
		Device,
		BufferSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		StagingBuffer,
		StagingBufferMemory);

	void* Data;
	if (vkMapMemory(Device, StagingBufferMemory, 0, BufferSize, 0, &Data) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to map buffer memory.");
	}
	memcpy(Data, Indices.data(), static_cast<size_t>(BufferSize));
	vkUnmapMemory(Device, StagingBufferMemory);

	Vk::CreateBuffer(
		PhysicalDevice,
		Device,
		BufferSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		IndexBuffer,
		IndexBufferMemory);

	Vk::CopyBuffer(Device, CommandPool, GraphicsQueue, StagingBuffer, IndexBuffer, BufferSize);

	vkDestroyBuffer(Device, StagingBuffer, nullptr);
	vkFreeMemory(Device, StagingBufferMemory, nullptr);
}

void FSingleObjectRenderer::CreateUniformBuffers()
{
	VkDeviceSize UniformBufferSize = sizeof(FUniformBufferObject);

	UniformBuffers.resize(MAX_CONCURRENT_FRAME);

	for (size_t Idx = 0; Idx < MAX_CONCURRENT_FRAME; ++Idx)
	{
		Vk::CreateBuffer(
			PhysicalDevice,
			Device,
			UniformBufferSize,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			UniformBuffers[Idx].Buffer,
			UniformBuffers[Idx].Memory);

		vkMapMemory(Device, UniformBuffers[Idx].Memory, 0, UniformBufferSize, 0, &UniformBuffers[Idx].Mapped);
	}
}

void FSingleObjectRenderer::CreateDescriptorPool()
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

	if (vkCreateDescriptorPool(Device, &DescriptorPoolCI, nullptr, &DescriptorPool) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create descriptor pool.");
	}
}

void FSingleObjectRenderer::CreateDescriptorSets()
{
	std::vector<VkDescriptorSetLayout> Layouts(MAX_CONCURRENT_FRAME, DescriptorSetLayout);
	VkDescriptorSetAllocateInfo DescriptorSetAllocInfo{};
	DescriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	DescriptorSetAllocInfo.descriptorPool = DescriptorPool;
	DescriptorSetAllocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_CONCURRENT_FRAME);
	DescriptorSetAllocInfo.pSetLayouts = Layouts.data();

	DescriptorSets.resize(MAX_CONCURRENT_FRAME);
	if (vkAllocateDescriptorSets(Device, &DescriptorSetAllocInfo, DescriptorSets.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate Descriptor sets!");
	}

	for (size_t Idx = 0; Idx < MAX_CONCURRENT_FRAME; Idx++)
	{
		VkDescriptorBufferInfo BufferInfo{};
		BufferInfo.buffer = UniformBuffers[Idx].Buffer;
		BufferInfo.offset = 0;
		BufferInfo.range = sizeof(FUniformBufferObject);

		VkDescriptorImageInfo ImageInfo{};
		ImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		ImageInfo.imageView = TextureImageView;
		ImageInfo.sampler = TextureSampler;

		std::array<VkWriteDescriptorSet, 2> DescriptorWrites{};

		DescriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		DescriptorWrites[0].dstSet = DescriptorSets[Idx];
		DescriptorWrites[0].dstBinding = 0;
		DescriptorWrites[0].dstArrayElement = 0;
		DescriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		DescriptorWrites[0].descriptorCount = 1;
		DescriptorWrites[0].pBufferInfo = &BufferInfo;

		DescriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		DescriptorWrites[1].dstSet = DescriptorSets[Idx];
		DescriptorWrites[1].dstBinding = 1;
		DescriptorWrites[1].dstArrayElement = 0;
		DescriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		DescriptorWrites[1].descriptorCount = 1;
		DescriptorWrites[1].pImageInfo = &ImageInfo;

		vkUpdateDescriptorSets(Device, static_cast<uint32_t>(DescriptorWrites.size()), DescriptorWrites.data(), 0, nullptr);
	}
}

void FSingleObjectRenderer::CreateCommandBuffers()
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

void FSingleObjectRenderer::CreateSyncObjects()
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

void FSingleObjectRenderer::WaitIdle()
{
	vkDeviceWaitIdle(Device);
}

void FSingleObjectRenderer::RecreateSwapchain()
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
	CreateFramebuffers();
}

void FSingleObjectRenderer::CleanupSwapchain()
{
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

void FSingleObjectRenderer::UpdateUniformBuffer()
{
	FCamera* Camera = GEngine->GetCamera();
	assert(Camera != nullptr);

	float FOVRadians = glm::radians(Camera->GetFOV());
	float AspectRatio = SwapchainExtent.width / (float)SwapchainExtent.height;

	FUniformBufferObject UBO{};
	UBO.Model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -2.0f));
	UBO.View = Camera->GetViewMatrix();
	UBO.Projection = glm::perspectiveRH(FOVRadians, AspectRatio, 0.1f, 10.0f);

	memcpy(UniformBuffers[GCurrentFrame].Mapped, &UBO, sizeof(FUniformBufferObject));
}

void FSingleObjectRenderer::Render(float InDeltaTime)
{
	vkWaitForFences(Device, 1, &Fences[GCurrentFrame], VK_TRUE, UINT64_MAX);

	uint32_t ImageIndex;
	VkResult AcquireResult = vkAcquireNextImageKHR(
		Device,
		Swapchain,
		UINT64_MAX,
		ImageAvailableSemaphores[GCurrentFrame],
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

	vkResetFences(Device, 1, &Fences[GCurrentFrame]);

	vkResetCommandBuffer(CommandBuffers[GCurrentFrame], 0);
	RecordCommandBuffer(CommandBuffers[GCurrentFrame], ImageIndex);

	VkSemaphore WaitSemaphores[] = { ImageAvailableSemaphores[GCurrentFrame] };
	VkPipelineStageFlags WaitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	VkSemaphore SignalSemaphores[] = { RenderFinishedSemaphores[GCurrentFrame] };

	VkSubmitInfo SubmitInfo{};
	SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	SubmitInfo.waitSemaphoreCount = 1;
	SubmitInfo.pWaitSemaphores = WaitSemaphores;
	SubmitInfo.pWaitDstStageMask = WaitStages;
	SubmitInfo.commandBufferCount = 1;
	SubmitInfo.pCommandBuffers = &CommandBuffers[GCurrentFrame];
	SubmitInfo.signalSemaphoreCount = 1;
	SubmitInfo.pSignalSemaphores = SignalSemaphores;

	if (vkQueueSubmit(GraphicsQueue, 1, &SubmitInfo, Fences[GCurrentFrame]) != VK_SUCCESS)
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
	PresentInfo.pImageIndices = &ImageIndex;

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

	GCurrentFrame = (GCurrentFrame + 1) % MAX_CONCURRENT_FRAME;
}