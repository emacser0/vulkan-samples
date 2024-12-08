#pragma once

#include "vulkan/vulkan.h"
#include "glfw/glfw3.h"

#include <vector>
#include <cassert>

#ifdef NDEBUG
#define VK_ASSERT(f) \
{ \
	f; \
} 
#else
#define VK_ASSERT(f) \
{ \
	assert(f == VK_SUCCESS); \
} 
#endif

namespace Vk
{
	bool SupportsValidationLayer(const std::vector<const char*> InValidationLayers);

	bool DeviceSupportsExtensions(VkPhysicalDevice InDevice, const std::vector<const char*> InDeviceExtensions);

	void FindQueueFamilies(VkPhysicalDevice InDevice, VkSurfaceKHR InSurface, uint32_t& OutGraphicsFamily, uint32_t& OutPresentFamily);

	void QuerySwapchainSupport(
		VkPhysicalDevice InDevice,
		VkSurfaceKHR InSurface,
		VkSurfaceCapabilitiesKHR& OutCapabilities,
		std::vector<VkSurfaceFormatKHR>& OutFormats,
		std::vector<VkPresentModeKHR>& OutPresentModes);

	bool IsDeviceSuitable(VkPhysicalDevice InDevice, VkSurfaceKHR InSurface, const std::vector<const char*> InDeviceExtensions);

	VkFormat FindDepthFormat(VkPhysicalDevice InPhysicalDevice);
	VkFormat FindSupportedFormat(VkPhysicalDevice InPhysicalDevice, const std::vector<VkFormat>& InCandidates, VkImageTiling InTiling, VkFormatFeatureFlags InFeatures);

	uint32_t FindMemoryType(VkPhysicalDevice InDevice, uint32_t InTypeFilter, VkMemoryPropertyFlags InProperties);

	void CreateBuffer(
		VkPhysicalDevice InPhysicalDevice,
		VkDevice InDevice,
		VkDeviceSize InSize,
		VkBufferUsageFlags InUsage,
		VkMemoryPropertyFlags InProperties,
		VkBuffer& OutBuffer,
		VkDeviceMemory& OutMemory);

	void CopyBuffer(
		VkDevice InDevice,
		VkCommandPool InCommandPool,
		VkQueue InCommandQueue,
		VkBuffer InSrcBuffer,
		VkBuffer InDstBuffer,
		VkDeviceSize InSize);

	VkCommandBuffer BeginOneTimeCommandBuffer(VkDevice InDevice, VkCommandPool InCommandPool);

	void EndOneTimeCommandBuffer(
		VkDevice InDevice,
		VkCommandPool InCommandPool,
		VkQueue InCommandQueue,
		VkCommandBuffer InCommandBuffer);

	void CopyBufferToImage(
		VkDevice InDevice,
		VkCommandPool InCommandPool,
		VkQueue InCommandQueue,
		VkBuffer InBuffer,
		VkImage InImage,
		uint32_t InMipLevel,
		uint32_t InArrayLayers,
		VkExtent3D InExtent);

	void TransitionImageLayout(
		VkDevice InDevice,
		VkCommandPool InCommandPool,
		VkQueue InCommandQueue,
		VkImage InImage,
		uint32_t InMipLevels,
		uint32_t InArrayLayers,
		VkFormat InFormat,
		VkImageLayout InOldLayout,
		VkImageLayout InNewLayout);

	void GetVertexInputBindings(std::vector<VkVertexInputBindingDescription>& OutDescs);
	void GetVertexInputAttributes(std::vector<VkVertexInputAttributeDescription>& OutDescs);
	VkPipelineVertexInputStateCreateInfo GetVertexInputStateCI(
		const std::vector<VkVertexInputBindingDescription>& InBindingDescs,
		const std::vector<VkVertexInputAttributeDescription>& InAttributeDescs);
	VkPipelineInputAssemblyStateCreateInfo GetInputAssemblyStateCI();
	VkPipelineViewportStateCreateInfo GetViewportStateCI();
	VkPipelineRasterizationStateCreateInfo GetRasterizationStateCI();
	VkPipelineMultisampleStateCreateInfo GetMultisampleStateCI();
	VkPipelineDepthStencilStateCreateInfo GetDepthStencilStateCI();
	VkPipelineColorBlendStateCreateInfo GetColorBlendStateCI();
	VkPipelineColorBlendAttachmentState GetColorBlendAttachment();
}
