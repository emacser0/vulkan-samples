#pragma once

#include "vulkan/vulkan.h"
#include "glfw/glfw3.h"

#include <vector>

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

	VkShaderModule CreateShaderModule(VkDevice InDevice, const std::vector<char>& InCode);

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

	void CreateImage(
		VkPhysicalDevice InPhysicalDevice,
		VkDevice InDevice,
		uint32_t InWidth,
		uint32_t InHeight,
		VkFormat InFormat,
		VkImageTiling InTiling,
		VkImageUsageFlags InUsage,
		VkMemoryPropertyFlags InProperties,
		VkImage& OutImage,
		VkDeviceMemory& OutImageMemory);

	VkImageView CreateImageView(
		VkDevice InDevice,
		VkImage InImage,
		VkFormat InFormat,
		VkImageAspectFlags InAspectFlags);

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
		uint32_t InWidth,
		uint32_t InHeight);

	void TransitionImageLayout(
		VkDevice InDevice,
		VkCommandPool InCommandPool,
		VkQueue InCommandQueue,
		VkImage InImage,
		VkFormat InFormat,
		VkImageLayout InOldLayout,
		VkImageLayout InNewLayout);

	VkFormat FindDepthFormat(VkPhysicalDevice InPhysicalDevice);
	VkFormat FindSupportedFormat(VkPhysicalDevice InPhysicalDevice, const std::vector<VkFormat>& InCandidates, VkImageTiling InTiling, VkFormatFeatureFlags InFeatures);
}
