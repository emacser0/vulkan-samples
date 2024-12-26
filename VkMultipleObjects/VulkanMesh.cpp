#include "VulkanMesh.h"
#include "VulkanContext.h"
#include "VulkanHelpers.h"
#include "VulkanTexture.h"

#include "MeshUtils.h"

#include <cassert>

FVulkanMesh::FVulkanMesh(FVulkanContext* InContext)
	: FVulkanObject(InContext)
	, Texture(nullptr)
	, bLoaded(false)
{

}

FVulkanMesh::~FVulkanMesh()
{
	VkDevice Device = Context->GetDevice();

	vkDestroyBuffer(Device, VertexBuffer.Buffer, nullptr);
	vkFreeMemory(Device, VertexBuffer.Memory, nullptr);

	vkDestroyBuffer(Device, IndexBuffer.Buffer, nullptr);
	vkFreeMemory(Device, IndexBuffer.Memory, nullptr);
}

bool FVulkanMesh::Load(const std::string& InFilename)
{
	Vertices.clear();
	Indices.clear();

	if (LoadModel(InFilename, Vertices, Indices) == false)
	{
		return false;
	}

	CreateVertexBuffer();
	CreateIndexBuffer();

	return true;
}

void FVulkanMesh::CreateVertexBuffer()
{
	VkPhysicalDevice PhysicalDevice = Context->GetPhysicalDevice();
	VkDevice Device = Context->GetDevice();
	VkQueue GfxQueue = Context->GetGfxQueue();
	VkCommandPool CommandPool = Context->GetCommandPool();

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
		VertexBuffer.Buffer,
		VertexBuffer.Memory);

	Vk::CopyBuffer(Device, CommandPool, GfxQueue, StagingBuffer, VertexBuffer.Buffer, BufferSize);

	vkDestroyBuffer(Device, StagingBuffer, nullptr);
	vkFreeMemory(Device, StagingBufferMemory, nullptr);
}

void FVulkanMesh::CreateIndexBuffer()
{
	VkPhysicalDevice PhysicalDevice = Context->GetPhysicalDevice();
	VkDevice Device = Context->GetDevice();
	VkQueue GfxQueue = Context->GetGfxQueue();
	VkCommandPool CommandPool = Context->GetCommandPool();

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
		IndexBuffer.Buffer,
		IndexBuffer.Memory);

	Vk::CopyBuffer(Device, CommandPool, GfxQueue, StagingBuffer, IndexBuffer.Buffer, BufferSize);

	vkDestroyBuffer(Device, StagingBuffer, nullptr);
	vkFreeMemory(Device, StagingBufferMemory, nullptr);
}
