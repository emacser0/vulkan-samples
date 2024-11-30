#include "VulkanMeshBase.h"
#include "VulkanContext.h"

#include "Vertex.h"
#include "Mesh.h"

#include <vector>

FVulkanMeshBase::FVulkanMeshBase(class FVulkanContext* InContext)
	: FVulkanObject(InContext)
	, MeshAsset(nullptr)
	, VertexBuffer()
	, IndexBuffer()
{
	VertexBuffer = InContext->CreateObject<FVulkanBuffer>();
	VertexBuffer->SetUsage(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
	VertexBuffer->SetProperties(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	IndexBuffer = InContext->CreateObject<FVulkanBuffer>();
	IndexBuffer->SetUsage(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
	IndexBuffer->SetProperties(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
}

void FVulkanMeshBase::Destroy()
{
	Unload();
}

bool FVulkanMeshBase::Load(UMesh* InMesh)
{
	if (InMesh == nullptr)
	{
		return false;
	}

	MeshAsset = InMesh;

	const std::vector<FVertex>& Vertices = InMesh->GetVertices();
	const std::vector<uint32_t>& Indices = InMesh->GetIndices();

	VertexBuffer->Load((uint8_t*)Vertices.data(), sizeof(FVertex) * Vertices.size());
	IndexBuffer->Load((uint8_t*)Indices.data(), sizeof(uint32_t) * Indices.size());

	return true;
}

void FVulkanMeshBase::Unload()
{
	VkDevice Device = Context->GetDevice();

	MeshAsset = nullptr;

	if (VertexBuffer)
	{
		VertexBuffer->Unload();
	}

	if (IndexBuffer)
	{
		IndexBuffer->Unload();
	}
}
