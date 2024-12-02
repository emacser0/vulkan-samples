#include "VulkanBuffer.h"
#include "VulkanContext.h"
#include "VulkanHelpers.h"

FVulkanBuffer::FVulkanBuffer(FVulkanContext* InContext)
	: FVulkanObject(InContext)
	, Buffer(VK_NULL_HANDLE)
	, Memory(VK_NULL_HANDLE)
	, Mapped(nullptr)
	, AllocatedSize(0)
	, Usage(0)
	, Properties(0)
{

}

void FVulkanBuffer::Destroy()
{
	Unload();
}

void FVulkanBuffer::Allocate(VkDeviceSize InBufferSize)
{
	VkPhysicalDevice PhysicalDevice = Context->GetPhysicalDevice();
	VkDevice Device = Context->GetDevice();

	Vk::CreateBuffer(
		PhysicalDevice,
		Device,
		InBufferSize,
		Usage,
		Properties,
		Buffer,
		Memory);

	AllocatedSize = InBufferSize;
}

void FVulkanBuffer::Unallocate()
{
	VkDevice Device = Context->GetDevice();

	if (Mapped != nullptr)
	{
		Unmap();
	}

	if (Buffer != VK_NULL_HANDLE)
	{
		vkDestroyBuffer(Device, Buffer, nullptr);
	}

	if (Memory != VK_NULL_HANDLE)
	{
		vkFreeMemory(Device, Memory, nullptr);
	}

	Buffer = VK_NULL_HANDLE;
	Memory = VK_NULL_HANDLE;
	AllocatedSize = 0;
}

void FVulkanBuffer::Load(uint8_t* InData, VkDeviceSize InBufferSize)
{
	Unload();
	Allocate(InBufferSize);
	Copy(InData, InBufferSize);
}

void FVulkanBuffer::Unload()
{
	Unallocate();
}

bool FVulkanBuffer::Copy(uint8_t* InData, VkDeviceSize InBufferSize)
{
	if (AllocatedSize != InBufferSize)
	{
		return false;
	}

	VkPhysicalDevice PhysicalDevice = Context->GetPhysicalDevice();
	VkDevice Device = Context->GetDevice();
	VkQueue GfxQueue = Context->GetGfxQueue();
	VkCommandPool CommandPool = Context->GetCommandPool();

	VkBuffer StagingBuffer;
	VkDeviceMemory StagingBufferMemory;
	Vk::CreateBuffer(
		PhysicalDevice,
		Device,
		InBufferSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		StagingBuffer,
		StagingBufferMemory);

	void* MappedMemory;
	VK_ASSERT(vkMapMemory(Device, StagingBufferMemory, 0, InBufferSize, 0, &MappedMemory));
	memcpy(MappedMemory, InData, static_cast<size_t>(InBufferSize));
	vkUnmapMemory(Device, StagingBufferMemory);

	Vk::CopyBuffer(Device, CommandPool, GfxQueue, StagingBuffer, Buffer, InBufferSize);

	vkDestroyBuffer(Device, StagingBuffer, nullptr);
	vkFreeMemory(Device, StagingBufferMemory, nullptr);

	return true;
}

void FVulkanBuffer::Map()
{
	if (Buffer == VK_NULL_HANDLE || Memory == VK_NULL_HANDLE)
	{
		return;
	}

	VkDevice Device = Context->GetDevice();
	VK_ASSERT(vkMapMemory(Device, Memory, 0, AllocatedSize, 0, &Mapped));
}

void FVulkanBuffer::Unmap()
{
	if (Mapped == nullptr)
	{
		return;
	}

	VkDevice Device = Context->GetDevice();
	vkUnmapMemory(Device, Memory);

	Mapped = nullptr;
}

