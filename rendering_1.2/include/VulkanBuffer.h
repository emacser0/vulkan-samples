#pragma once

#include "VulkanObject.h"

#include "vulkan/vulkan.h"

#include <vector>

class FVulkanBuffer : public FVulkanObject
{
public:
	FVulkanBuffer(class FVulkanContext* InContext);

	virtual void Destroy() override;

	VkBuffer GetBuffer() const { return Buffer; }
	VkDeviceMemory GetMemory() const { return Memory; }
	void* GetMappedAddress() const { return Mapped; }

	void SetUsage(VkBufferUsageFlags InUsage) { Usage = InUsage; }
	void SetProperties(VkMemoryPropertyFlags InProperties) { Properties = InProperties; }

	void Allocate(VkDeviceSize InBufferSize);
	void Unallocate();

	void Load(uint8_t* InData, VkDeviceSize InBufferSize);
	void Unload();

	bool Copy(uint8_t* InData, VkDeviceSize InBufferSize);

	void Map();
	void Unmap();

protected:
	VkBuffer Buffer;
	VkDeviceMemory Memory;
	void* Mapped;

	VkDeviceSize AllocatedSize;

	VkBufferUsageFlags Usage;
	VkMemoryPropertyFlags Properties;
};
