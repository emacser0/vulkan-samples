#include "VulkanHelpers.h"
#include "Utils.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <set>
#include <string>
#include <stdexcept>

namespace Vk
{
	bool SupportsValidationLayer(const std::vector<const char*> InValidationLayers)
	{
		uint32_t LayerCount;
		vkEnumerateInstanceLayerProperties(&LayerCount, nullptr);

		std::vector<VkLayerProperties> AvailableLayers(LayerCount);
		vkEnumerateInstanceLayerProperties(&LayerCount, AvailableLayers.data());

		for (const char* LayerName : InValidationLayers)
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

	bool DeviceSupportsExtensions(VkPhysicalDevice InDevice, const std::vector<const char*> InDeviceExtensions)
	{
		uint32_t ExtensionCount;
		vkEnumerateDeviceExtensionProperties(InDevice, nullptr, &ExtensionCount, nullptr);

		if (ExtensionCount == 0)
		{
			return InDeviceExtensions.empty();
		}

		std::vector<VkExtensionProperties> AvailableExtensions(ExtensionCount);
		vkEnumerateDeviceExtensionProperties(InDevice, nullptr, &ExtensionCount, AvailableExtensions.data());

		for (const char* RequiredExtension : InDeviceExtensions)
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

	void FindQueueFamilies(VkPhysicalDevice InDevice, VkSurfaceKHR InSurface, uint32_t& OutGraphicsFamily, uint32_t& OutPresentFamily)
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
			vkGetPhysicalDeviceSurfaceSupportKHR(InDevice, Idx, InSurface, &PresentSupport);

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
		VkSurfaceKHR InSurface,
		VkSurfaceCapabilitiesKHR& OutCapabilities,
		std::vector<VkSurfaceFormatKHR>& OutFormats,
		std::vector<VkPresentModeKHR>& OutPresentModes)
	{
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(InDevice, InSurface, &OutCapabilities);

		uint32_t FormatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(InDevice, InSurface, &FormatCount, nullptr);
		
		if (FormatCount > 0)
		{
			OutFormats.resize(FormatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(InDevice, InSurface, &FormatCount, OutFormats.data());
		}

		uint32_t PresentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(InDevice, InSurface, &PresentModeCount, nullptr);

		if (PresentModeCount > 0)
		{
			OutPresentModes.resize(PresentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(InDevice, InSurface, &PresentModeCount, OutPresentModes.data());
		}
	}

	bool IsDeviceSuitable(VkPhysicalDevice InDevice, VkSurfaceKHR InSurface, const std::vector<const char*> InDeviceExtensions)
	{
		uint32_t GraphicsFamily = -1;
		uint32_t PresentFamily = -1;

		FindQueueFamilies(InDevice, InSurface, GraphicsFamily, PresentFamily);

		bool bExtensionsSupported = DeviceSupportsExtensions(InDevice, InDeviceExtensions);

		bool bSwapchainAdequate = false;
		if (bExtensionsSupported)
		{
			VkSurfaceCapabilitiesKHR Capabilities;
			std::vector<VkSurfaceFormatKHR> Formats;
			std::vector<VkPresentModeKHR> PresentModes;

			QuerySwapchainSupport(InDevice, InSurface, Capabilities, Formats, PresentModes);
			bSwapchainAdequate = !Formats.empty() && !PresentModes.empty();
		}

		return GraphicsFamily != -1 && PresentFamily != -1 && bExtensionsSupported && bSwapchainAdequate;
	}

	VkFormat FindDepthFormat(VkPhysicalDevice InPhysicalDevice)
	{
		return FindSupportedFormat(
			InPhysicalDevice,
			{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
	}

	VkFormat FindSupportedFormat(VkPhysicalDevice InPhysicalDevice, const std::vector<VkFormat>& InCandidates, VkImageTiling InTiling, VkFormatFeatureFlags InFeatures)
	{
		for (VkFormat Format : InCandidates)
		{
			VkFormatProperties Properties;
			vkGetPhysicalDeviceFormatProperties(InPhysicalDevice, Format, &Properties);

			if (InTiling == VK_IMAGE_TILING_LINEAR && (Properties.linearTilingFeatures & InFeatures) == InFeatures)
			{
				return Format;
			}
			else if (InTiling == VK_IMAGE_TILING_OPTIMAL && (Properties.optimalTilingFeatures & InFeatures) == InFeatures)
			{
				return Format;
			}

		}

		std::runtime_error("Failed to find supported format.");

		return VK_FORMAT_UNDEFINED;
	}

	uint32_t FindMemoryType(VkPhysicalDevice InPhysicalDevice, uint32_t InTypeFilter, VkMemoryPropertyFlags InProperties)
	{
		VkPhysicalDeviceMemoryProperties MemoryProperties;
		vkGetPhysicalDeviceMemoryProperties(InPhysicalDevice, &MemoryProperties);

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
		VkPhysicalDevice InPhysicalDevice,
		VkDevice InDevice,
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

		if (vkCreateBuffer(InDevice, &BufferCI, nullptr, &OutBuffer) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create buffer.");
		}

		VkMemoryRequirements MemoryReqs{};
		vkGetBufferMemoryRequirements(InDevice, OutBuffer, &MemoryReqs);

		VkMemoryAllocateInfo MemoryAllocInfo{};
		MemoryAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		MemoryAllocInfo.allocationSize = MemoryReqs.size;
		MemoryAllocInfo.memoryTypeIndex = FindMemoryType(InPhysicalDevice, MemoryReqs.memoryTypeBits, InProperties);

		if (vkAllocateMemory(InDevice, &MemoryAllocInfo, nullptr, &OutMemory) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to allocate buffer memory.");
		}

		if (vkBindBufferMemory(InDevice, OutBuffer, OutMemory, 0) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to bind buffer memory.");
		}
	}

	void CopyBuffer(
		VkDevice InDevice,
		VkCommandPool InCommandPool,
		VkQueue InCommandQueue,
		VkBuffer InSrcBuffer,
		VkBuffer InDstBuffer,
		VkDeviceSize InSize)
	{
		VkCommandBufferAllocateInfo CommandBufferAllocInfo{};
		CommandBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		CommandBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		CommandBufferAllocInfo.commandPool = InCommandPool;
		CommandBufferAllocInfo.commandBufferCount = 1;

		VkCommandBuffer CommandBuffer{};
		vkAllocateCommandBuffers(InDevice, &CommandBufferAllocInfo, &CommandBuffer);

		VkCommandBufferBeginInfo CommandBufferBeginInfo{};
		CommandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		CommandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		if (vkBeginCommandBuffer(CommandBuffer, &CommandBufferBeginInfo) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to begin command buffer.");
		}

		VkBufferCopy CopyRegion{};
		CopyRegion.size = InSize;
		vkCmdCopyBuffer(CommandBuffer, InSrcBuffer, InDstBuffer, 1, &CopyRegion);

		vkEndCommandBuffer(CommandBuffer);

		VkSubmitInfo SubmitInfo{};
		SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		SubmitInfo.commandBufferCount = 1;
		SubmitInfo.pCommandBuffers = &CommandBuffer;

		vkQueueSubmit(InCommandQueue, 1, &SubmitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(InCommandQueue);

		vkFreeCommandBuffers(InDevice, InCommandPool, 1, &CommandBuffer);
	}

	void CreateImage(
		VkPhysicalDevice InPhysicalDevice,
		VkDevice InDevice,
		uint32_t InWidth,
		uint32_t InHeight,
		uint32_t InDepth,
		VkFormat InFormat,
		VkImageType InImageType,
		VkImageTiling InTiling,
		VkImageUsageFlags InUsage,
		VkMemoryPropertyFlags InProperties,
		VkImage& OutImage,
		VkDeviceMemory& OutImageMemory)
	{
		VkImageCreateInfo ImageCI{};
		ImageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		ImageCI.imageType = InImageType;
		ImageCI.extent.width = InWidth;
		ImageCI.extent.height = InHeight;
		ImageCI.extent.depth = InDepth;
		ImageCI.mipLevels = 1;
		ImageCI.arrayLayers = 1;
		ImageCI.format = InFormat;
		ImageCI.tiling = InTiling;
		ImageCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		ImageCI.usage = InUsage;
		ImageCI.samples = VK_SAMPLE_COUNT_1_BIT;
		ImageCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateImage(InDevice, &ImageCI, nullptr, &OutImage) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create image.");
		}

		VkMemoryRequirements MemoryReqs{};
		vkGetImageMemoryRequirements(InDevice, OutImage, &MemoryReqs);

		VkMemoryAllocateInfo MemoryAllocInfo{};
		MemoryAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		MemoryAllocInfo.allocationSize = MemoryReqs.size;
		MemoryAllocInfo.memoryTypeIndex = FindMemoryType(InPhysicalDevice, MemoryReqs.memoryTypeBits, InProperties);

		if (vkAllocateMemory(InDevice, &MemoryAllocInfo, nullptr, &OutImageMemory) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to allocate image memory.");
		}

		if (vkBindImageMemory(InDevice, OutImage, OutImageMemory, 0) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to bind image memory.");
		}
	}

	VkImageView CreateImageView(VkDevice InDevice, VkImage InImage, VkFormat InFormat, VkImageAspectFlags InAspectFlags)
	{
		VkImageViewCreateInfo ImageViewCI{};
		ImageViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		ImageViewCI.image = InImage;
		ImageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
		ImageViewCI.format = InFormat;
		ImageViewCI.subresourceRange.aspectMask = InAspectFlags;
		ImageViewCI.subresourceRange.baseMipLevel = 0;
		ImageViewCI.subresourceRange.levelCount = 1;
		ImageViewCI.subresourceRange.baseArrayLayer = 0;
		ImageViewCI.subresourceRange.layerCount = 1;

		VkImageView ImageView;
		if (vkCreateImageView(InDevice, &ImageViewCI, nullptr, &ImageView) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create image view.");
		}

		return ImageView;
	}

	VkCommandBuffer BeginOneTimeCommandBuffer(VkDevice InDevice, VkCommandPool InCommandPool)
	{
		VkCommandBufferAllocateInfo CommandBufferAllocInfo{};
		CommandBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		CommandBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		CommandBufferAllocInfo.commandPool = InCommandPool;
		CommandBufferAllocInfo.commandBufferCount = 1;

		VkCommandBuffer CommandBuffer;
		if (vkAllocateCommandBuffers(InDevice, &CommandBufferAllocInfo, &CommandBuffer) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to allocate command buffer.");
		}

		VkCommandBufferBeginInfo CommandBufferBeginInfo{};
		CommandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		CommandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		if (vkBeginCommandBuffer(CommandBuffer, &CommandBufferBeginInfo) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to begin command buffer.");
		}

		return CommandBuffer;
	}

	void EndOneTimeCommandBuffer(VkDevice InDevice, VkCommandPool InCommandPool, VkQueue InCommandQueue, VkCommandBuffer InCommandBuffer)
	{
		vkEndCommandBuffer(InCommandBuffer);

		VkSubmitInfo SubmitInfo{};
		SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		SubmitInfo.commandBufferCount = 1;
		SubmitInfo.pCommandBuffers = &InCommandBuffer;

		vkQueueSubmit(InCommandQueue, 1, &SubmitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(InCommandQueue);

		vkFreeCommandBuffers(InDevice, InCommandPool, 1, &InCommandBuffer);
	}

	void CopyBufferToImage(
		VkDevice InDevice,
		VkCommandPool InCommandPool,
		VkQueue InCommandQueue,
		VkBuffer InBuffer,
		VkImage InImage,
		uint32_t InWidth,
		uint32_t InHeight,
		uint32_t InDepth)
	{
		VkCommandBuffer CommandBuffer = BeginOneTimeCommandBuffer(InDevice, InCommandPool);

		VkBufferImageCopy CopyRegion{};
		CopyRegion.bufferOffset = 0;
		CopyRegion.bufferRowLength = 0;
		CopyRegion.bufferImageHeight = 0;
		CopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		CopyRegion.imageSubresource.mipLevel = 0;
		CopyRegion.imageSubresource.baseArrayLayer = 0;
		CopyRegion.imageSubresource.layerCount = 1;
		CopyRegion.imageOffset = { 0, 0, 0 };
		CopyRegion.imageExtent = { InWidth, InHeight, InDepth };

		vkCmdCopyBufferToImage(CommandBuffer, InBuffer, InImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &CopyRegion);

		EndOneTimeCommandBuffer(InDevice, InCommandPool, InCommandQueue, CommandBuffer);
	}

	void TransitionImageLayout(
		VkDevice InDevice,
		VkCommandPool InCommandPool,
		VkQueue InCommandQueue,
		VkImage InImage,
		VkFormat InFormat,
		VkImageLayout InOldLayout,
		VkImageLayout InNewLayout)
	{
		VkCommandBuffer CommandBuffer = BeginOneTimeCommandBuffer(InDevice, InCommandPool);

		VkImageMemoryBarrier ImageMemoryBarrier{};
		ImageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		ImageMemoryBarrier.oldLayout = InOldLayout;
		ImageMemoryBarrier.newLayout = InNewLayout;
		ImageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		ImageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		ImageMemoryBarrier.image = InImage;
		ImageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		ImageMemoryBarrier.subresourceRange.baseMipLevel = 0;
		ImageMemoryBarrier.subresourceRange.levelCount = 1;
		ImageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
		ImageMemoryBarrier.subresourceRange.layerCount = 1;

		VkPipelineStageFlags SrcStage;
		VkPipelineStageFlags DstStage;

		if (InOldLayout == VK_IMAGE_LAYOUT_UNDEFINED && InNewLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			ImageMemoryBarrier.srcAccessMask = 0;
			ImageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			SrcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			DstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (InOldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && InNewLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			ImageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			ImageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			SrcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			DstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else
		{
			throw std::runtime_error("Unsupported layout transition.");
		}

		vkCmdPipelineBarrier(
			CommandBuffer,
			SrcStage, DstStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &ImageMemoryBarrier);

		EndOneTimeCommandBuffer(InDevice, InCommandPool, InCommandQueue, CommandBuffer);
	}

	VkPipelineVertexInputStateCreateInfo GetVertexInputStateCI(
		const std::vector<VkVertexInputBindingDescription>& InBindingDescs,
		const std::vector<VkVertexInputAttributeDescription>& InAttributeDescs)
	{
		VkPipelineVertexInputStateCreateInfo VertexInputStateCI{};
		VertexInputStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		VertexInputStateCI.vertexBindingDescriptionCount = static_cast<uint32_t>(InBindingDescs.size());
		VertexInputStateCI.pVertexBindingDescriptions = InBindingDescs.data();
		VertexInputStateCI.vertexAttributeDescriptionCount = static_cast<uint32_t>(InAttributeDescs.size());
		VertexInputStateCI.pVertexAttributeDescriptions = InAttributeDescs.data();

		return VertexInputStateCI;
	}

	VkPipelineInputAssemblyStateCreateInfo GetInputAssemblyStateCI()
	{
		VkPipelineInputAssemblyStateCreateInfo InputAssemblyStateCI{};
		InputAssemblyStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		InputAssemblyStateCI.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		InputAssemblyStateCI.primitiveRestartEnable = VK_FALSE;

		return InputAssemblyStateCI;
	}

	VkPipelineViewportStateCreateInfo GetViewportStateCI()
	{
		VkPipelineViewportStateCreateInfo ViewportStateCI{};
		ViewportStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		ViewportStateCI.viewportCount = 1;
		ViewportStateCI.scissorCount = 1;

		return ViewportStateCI;
	}

	VkPipelineRasterizationStateCreateInfo GetRasterizationStateCI()
	{
		VkPipelineRasterizationStateCreateInfo RasterizationStateCI{};
		RasterizationStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		RasterizationStateCI.depthClampEnable = VK_FALSE;
		RasterizationStateCI.rasterizerDiscardEnable = VK_FALSE;
		RasterizationStateCI.polygonMode = VK_POLYGON_MODE_FILL;
		RasterizationStateCI.lineWidth = 1.0f;
		RasterizationStateCI.cullMode = VK_CULL_MODE_BACK_BIT;
		RasterizationStateCI.frontFace = VK_FRONT_FACE_CLOCKWISE;
		RasterizationStateCI.depthBiasEnable = VK_FALSE;

		return RasterizationStateCI;
	}

	VkPipelineMultisampleStateCreateInfo GetMultisampleStateCI()
	{
		VkPipelineMultisampleStateCreateInfo MultisampleStateCI{};
		MultisampleStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		MultisampleStateCI.sampleShadingEnable = VK_FALSE;
		MultisampleStateCI.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		MultisampleStateCI.flags = 0;

		return MultisampleStateCI;
	}

	VkPipelineDepthStencilStateCreateInfo GetDepthStencilStateCI()
	{
		VkPipelineDepthStencilStateCreateInfo DepthStencilStateCI{};
		DepthStencilStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		DepthStencilStateCI.depthTestEnable = VK_TRUE;
		DepthStencilStateCI.depthWriteEnable = VK_TRUE;
		DepthStencilStateCI.depthCompareOp = VK_COMPARE_OP_LESS;
		DepthStencilStateCI.depthBoundsTestEnable = VK_FALSE;
		DepthStencilStateCI.stencilTestEnable = VK_FALSE;

		return DepthStencilStateCI;
	}

	VkPipelineColorBlendStateCreateInfo GetColorBlendStateCI()
	{
		VkPipelineColorBlendStateCreateInfo ColorBlendStateCI{};
		ColorBlendStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		ColorBlendStateCI.logicOpEnable = VK_FALSE;
		ColorBlendStateCI.logicOp = VK_LOGIC_OP_COPY;
		ColorBlendStateCI.attachmentCount = 1;
		ColorBlendStateCI.blendConstants[0] = 0.0f;
		ColorBlendStateCI.blendConstants[1] = 0.0f;
		ColorBlendStateCI.blendConstants[2] = 0.0f;
		ColorBlendStateCI.blendConstants[3] = 0.0f;

		return ColorBlendStateCI;
	}

	VkPipelineColorBlendAttachmentState GetColorBlendAttachment()
	{
		VkPipelineColorBlendAttachmentState ColorBlendAttachmentState{};
		ColorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		ColorBlendAttachmentState.blendEnable = VK_FALSE;

		return ColorBlendAttachmentState;
	}
}
