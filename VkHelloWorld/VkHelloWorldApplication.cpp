#include "VkHelloWorldApplication.h"
#include "Engine.h"
#include "Config.h"
#include "Utils.h"
#include "VulkanHelpers.h"

#include "glfw/glfw3.h"

#include <iostream>
#include <set>
#include <algorithm>

#ifdef NDEBUG
#define ENABLE_VALIDATION_LAYERS 0
#else
#define ENABLE_VALIDATION_LAYERS 1
#endif

#define MAX_CONCURRENT_FRAME 2

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

FVkHelloWorldApplication::FVkHelloWorldApplication()
	: ValidationLayers({ "VK_LAYER_KHRONOS_validation" })
	, DeviceExtensions({ VK_KHR_SWAPCHAIN_EXTENSION_NAME })
	, Instance(VK_NULL_HANDLE)
	, DebugMessenger(VK_NULL_HANDLE)
	, Surface(VK_NULL_HANDLE)
	, PhysicalDevice(VK_NULL_HANDLE)
	, Device(VK_NULL_HANDLE)
	, GraphicsQueue(VK_NULL_HANDLE)
	, PresentQueue(VK_NULL_HANDLE)
	, Swapchain(VK_NULL_HANDLE)
	, SwapchainImageFormat()
	, SwapchainExtent({})
	, RenderPass(VK_NULL_HANDLE)
	, PipelineLayout(VK_NULL_HANDLE)
	, Pipeline(VK_NULL_HANDLE)
	, CommandPool(VK_NULL_HANDLE)
	, VertexBuffer(VK_NULL_HANDLE)
	, VertexBufferMemory(VK_NULL_HANDLE)
	, IndexBuffer(VK_NULL_HANDLE)
	, IndexBufferMemory(VK_NULL_HANDLE)
	, CurrentFrame(0)
	, Vertices({
		{ { -0.5f, 0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
		{ { 0.0f, -0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f } },
		{ { 0.5f, 0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f } } })
	, Indices({ 0, 1, 2 })
{
}

void FVkHelloWorldApplication::Run()
{
	AddResizeCallback();
	CreateInstance();
	SetupDebugMessenger();
	CreateSurface();
	PickPhysicalDevice();
	CreateLogicalDevice();
	CreateSwapchain();
	CreateImageViews();
	CreateRenderPass();
	CreateGraphicsPipeline();
	CreateFramebuffers();
	CreateCommandPool();
	CreateVertexBuffer();
	CreateIndexBuffer();
	CreateCommandBuffers();
	CreateSyncObjects();
}

void FVkHelloWorldApplication::Terminate()
{
	WaitIdle();
	Cleanup();
}

void FVkHelloWorldApplication::Tick(float InDeltaTime)
{
	Render();
}

void FVkHelloWorldApplication::AddResizeCallback()
{
	GLFWwindow* Window = GEngine->GetWindow();
	assert(Window != nullptr);

	glfwSetFramebufferSizeCallback(Window, FramebufferResizeCallback);
}

void FVkHelloWorldApplication::CreateInstance()
{
	if (ENABLE_VALIDATION_LAYERS && !Vk::SupportsValidationLayer(ValidationLayers))
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

	if (ENABLE_VALIDATION_LAYERS)
	{
		InstanceCI.enabledLayerCount = static_cast<uint32_t>(ValidationLayers.size());
		InstanceCI.ppEnabledLayerNames = ValidationLayers.data();

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

	VK_ASSERT(vkCreateInstance(&InstanceCI, nullptr, &Instance));
}

void FVkHelloWorldApplication::SetupDebugMessenger()
{
	if (ENABLE_VALIDATION_LAYERS)
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

void FVkHelloWorldApplication::CreateSurface()
{
	GLFWwindow* Window = GEngine->GetWindow();
	assert(Window != nullptr);

	if (glfwCreateWindowSurface(Instance, Window, nullptr, &Surface) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create window surface.");
	}
}

void FVkHelloWorldApplication::PickPhysicalDevice()
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

void FVkHelloWorldApplication::CreateLogicalDevice()
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

	VkDeviceCreateInfo DeviceCI{};
	DeviceCI.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	DeviceCI.queueCreateInfoCount = static_cast<uint32_t>(QueueCIs.size());
	DeviceCI.pQueueCreateInfos = QueueCIs.data();

	DeviceCI.pEnabledFeatures = &DeviceFeatures;

	DeviceCI.enabledExtensionCount = static_cast<uint32_t>(DeviceExtensions.size());
	DeviceCI.ppEnabledExtensionNames = DeviceExtensions.data();

	if (ENABLE_VALIDATION_LAYERS)
	{
		DeviceCI.enabledLayerCount = static_cast<uint32_t>(ValidationLayers.size());
		DeviceCI.ppEnabledLayerNames = ValidationLayers.data();
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

void FVkHelloWorldApplication::CreateSwapchain()
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
		GLFWwindow* Window = GEngine->GetWindow();
		assert(Window != nullptr);

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

	VK_ASSERT(vkCreateSwapchainKHR(Device, &SwapchainCI, nullptr, &Swapchain));

	uint32_t SwapchainImageCount;
	vkGetSwapchainImagesKHR(Device, Swapchain, &SwapchainImageCount, nullptr);

	SwapchainImages.resize(SwapchainImageCount);
	vkGetSwapchainImagesKHR(Device, Swapchain, &SwapchainImageCount, SwapchainImages.data());

	SwapchainImageFormat = ChoosenFormat.format;
	SwapchainExtent = ChoosenSwapchainExtent;
}

void FVkHelloWorldApplication::CreateImageViews()
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

		VK_ASSERT(vkCreateImageView(Device, &ImageViewCI, nullptr, &SwapchainImageViews[Idx]));
	}
}

void FVkHelloWorldApplication::CreateRenderPass()
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

	VK_ASSERT(vkCreateRenderPass(Device, &RenderPassCI, nullptr, &RenderPass));
}

void FVkHelloWorldApplication::CreateGraphicsPipeline()
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

	VK_ASSERT(vkCreatePipelineLayout(Device, &PipelineLayoutCI, nullptr, &PipelineLayout));

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

	VK_ASSERT(vkCreateGraphicsPipelines(Device, VK_NULL_HANDLE, 1, &PipelineCI, nullptr, &Pipeline));

	vkDestroyShaderModule(Device, VertexShaderModule, nullptr);
	vkDestroyShaderModule(Device, FragmentShaderModule, nullptr);
}

void FVkHelloWorldApplication::CreateFramebuffers()
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

		VK_ASSERT(vkCreateFramebuffer(Device, &FramebufferCI, nullptr, &SwapchainFramebuffers[Idx]));
	}
}

void FVkHelloWorldApplication::CreateCommandPool()
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

void FVkHelloWorldApplication::CreateVertexBuffer()
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
	VK_ASSERT(vkMapMemory(Device, StagingBufferMemory, 0, BufferSize, 0, &Data));
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

void FVkHelloWorldApplication::CreateIndexBuffer()
{
	VkDeviceSize BufferSize = sizeof(uint32_t) * Indices.size();

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
	VK_ASSERT(vkMapMemory(Device, StagingBufferMemory, 0, BufferSize, 0, &Data));
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

void FVkHelloWorldApplication::CreateCommandBuffers()
{
	CommandBuffers.resize(MAX_CONCURRENT_FRAME);

	VkCommandBufferAllocateInfo CommandBufferAllocInfo{};
	CommandBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	CommandBufferAllocInfo.commandPool = CommandPool;
	CommandBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	CommandBufferAllocInfo.commandBufferCount = static_cast<uint32_t>(CommandBuffers.size());

	VK_ASSERT(vkAllocateCommandBuffers(Device, &CommandBufferAllocInfo, CommandBuffers.data()));
}

void FVkHelloWorldApplication::CreateSyncObjects()
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
		VK_ASSERT(vkCreateSemaphore(Device, &SemaphoreCI, nullptr, &ImageAvailableSemaphores[Idx]));
		VK_ASSERT(vkCreateSemaphore(Device, &SemaphoreCI, nullptr, &RenderFinishedSemaphores[Idx]));
		VK_ASSERT(vkCreateFence(Device, &FenceCI, nullptr, &Fences[Idx]));
	}
}

void FVkHelloWorldApplication::WaitIdle()
{
	vkDeviceWaitIdle(Device);
}

void FVkHelloWorldApplication::RecordCommandBuffer(VkCommandBuffer InCommandBuffer, uint32_t InImageIndex)
{
	VkCommandBufferBeginInfo CommandBufferBeginInfo{};
	CommandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	VK_ASSERT(vkBeginCommandBuffer(InCommandBuffer, &CommandBufferBeginInfo));

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

	vkCmdBindIndexBuffer(InCommandBuffer, IndexBuffer, 0, VK_INDEX_TYPE_UINT32);

	vkCmdDrawIndexed(InCommandBuffer, static_cast<uint32_t>(Indices.size()), 1, 0, 0, 0);

	vkCmdEndRenderPass(InCommandBuffer);

	VK_ASSERT(vkEndCommandBuffer(InCommandBuffer));
}

void FVkHelloWorldApplication::RecreateSwapchain()
{
	GLFWwindow* Window = GEngine->GetWindow();
	assert(Window != nullptr);

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

void FVkHelloWorldApplication::CleanupSwapchain()
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

void FVkHelloWorldApplication::Render()
{
	vkWaitForFences(Device, 1, &Fences[CurrentFrame], VK_TRUE, UINT64_MAX);

	uint32_t ImageIndex;
	VkResult AcquireResult = vkAcquireNextImageKHR(
		Device,
		Swapchain,
		UINT64_MAX,
		ImageAvailableSemaphores[CurrentFrame],
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

	vkResetFences(Device, 1, &Fences[CurrentFrame]);

	vkResetCommandBuffer(CommandBuffers[CurrentFrame], 0);
	RecordCommandBuffer(CommandBuffers[CurrentFrame], ImageIndex);

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

	VK_ASSERT(vkQueueSubmit(GraphicsQueue, 1, &SubmitInfo, Fences[CurrentFrame]));

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

	CurrentFrame = (CurrentFrame + 1) % MAX_CONCURRENT_FRAME;
}

void FVkHelloWorldApplication::Cleanup()
{
	CleanupSwapchain();

	vkDestroyPipelineLayout(Device, PipelineLayout, nullptr);
	vkDestroyPipeline(Device, Pipeline, nullptr);

	vkDestroyRenderPass(Device, RenderPass, nullptr);

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

	if (ENABLE_VALIDATION_LAYERS)
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
