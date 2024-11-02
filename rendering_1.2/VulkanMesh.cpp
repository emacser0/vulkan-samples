#include "VulkanMesh.h"
#include "VulkanContext.h"
#include "VulkanHelpers.h"
#include "VulkanTexture.h"

#include "Mesh.h"

#include "glm/glm.hpp"

FVulkanMesh::FVulkanMesh(FVulkanContext* InContext)
	: FVulkanObject(InContext)
	, MeshAsset(nullptr)
	, BaseColorTexture(nullptr)
	, NormalTexture(nullptr)
	, bLoaded(false)
{

}

FVulkanMesh::~FVulkanMesh()
{
	Unload();
}

bool FVulkanMesh::Load(FMesh* InMesh)
{
	if (InMesh == nullptr)
	{
		return false;
	}

	MeshAsset = InMesh;

	const std::vector<FVertex>& Vertices = InMesh->GetVertices();
	const std::vector<uint32_t>& Indices = InMesh->GetIndices();

	CreateVertexBuffer(Vertices);
	CreateIndexBuffer(Indices);

	return true;
}

void FVulkanMesh::Unload()
{
	VkDevice Device = Context->GetDevice();

	MeshAsset = nullptr;

	if (VertexBuffer.Buffer != VK_NULL_HANDLE)
	{
		vkDestroyBuffer(Device, VertexBuffer.Buffer, nullptr);
	}

	if (VertexBuffer.Memory != VK_NULL_HANDLE)
	{
		vkFreeMemory(Device, VertexBuffer.Memory, nullptr);
	}

	if (IndexBuffer.Buffer != VK_NULL_HANDLE)
	{
		vkDestroyBuffer(Device, IndexBuffer.Buffer, nullptr);
	}

	if (IndexBuffer.Memory != VK_NULL_HANDLE)
	{
		vkFreeMemory(Device, IndexBuffer.Memory, nullptr);
	}

	VertexBuffer.Buffer = VK_NULL_HANDLE;
	VertexBuffer.Memory = VK_NULL_HANDLE;
	VertexBuffer.Mapped = nullptr;

	IndexBuffer.Buffer = VK_NULL_HANDLE;
	IndexBuffer.Memory = VK_NULL_HANDLE;
	IndexBuffer.Mapped = nullptr;
}

void FVulkanMesh::CreateVertexBuffer(const std::vector<FVertex>& Vertices)
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

void FVulkanMesh::CreateIndexBuffer(const std::vector<uint32_t>& Indices)
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

