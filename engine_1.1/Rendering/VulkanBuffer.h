#pragma once

#include "vulkan/vulkan.h"

struct FVulkanBuffer
{
	VkBuffer Buffer = VK_NULL_HANDLE;
	VkDeviceMemory Memory = VK_NULL_HANDLE;
	void* Mapped = nullptr;
};
