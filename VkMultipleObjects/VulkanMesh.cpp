#include "VulkanMesh.h"
#include "VulkanContext.h"
#include "VulkanHelpers.h"
#include "VulkanTexture.h"

#include <cassert>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

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
	tinyobj::attrib_t Attributes;
	std::vector<tinyobj::shape_t> Shapes;
	std::vector<tinyobj::material_t> Materials;
	std::string Warn, Error;

	if (tinyobj::LoadObj(&Attributes, &Shapes, &Materials, &Warn, &Error, InFilename.c_str()) == false)
	{
		return false;
	}

	Vertices.clear();
	Indices.clear();

	std::unordered_map<FVertex, uint32_t> UniqueVertices;

	for (const tinyobj::shape_t& Shape : Shapes)
	{
		for (const tinyobj::index_t& Index : Shape.mesh.indices)
		{
			FVertex NewVertex{};
			NewVertex.Position = glm::vec3(
				Attributes.vertices[3 * Index.vertex_index + 0],
				Attributes.vertices[3 * Index.vertex_index + 1],
				Attributes.vertices[3 * Index.vertex_index + 2]);
			NewVertex.Color = glm::vec3(1.0f, 1.0f, 1.0f);
			NewVertex.TexCoords = glm::vec2(
				Attributes.texcoords[2 * Index.texcoord_index + 0],
				1.0f - Attributes.texcoords[2 * Index.texcoord_index + 1]);

			const auto Iter = UniqueVertices.find(NewVertex);
			if (Iter == UniqueVertices.end())
			{
				UniqueVertices[NewVertex] = UniqueVertices.size();
				Vertices.push_back(NewVertex);
			}

			Indices.push_back(UniqueVertices[NewVertex]);
		}
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
