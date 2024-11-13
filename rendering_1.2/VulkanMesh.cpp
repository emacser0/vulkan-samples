#include "VulkanMesh.h"
#include "VulkanContext.h"
#include "VulkanHelpers.h"
#include "VulkanTexture.h"

#include "Mesh.h"

#include "glm/glm.hpp"

FVulkanMesh::FVulkanMesh(FVulkanContext* InContext)
	: FVulkanMeshBase(InContext)
	, BaseColorTexture(nullptr)
	, NormalTexture(nullptr)
	, bLoaded(false)
{

}

void FVulkanMesh::Destroy()
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

	VertexBuffer->Load((uint8_t*)Vertices.data(), sizeof(FVertex) * Vertices.size());
	IndexBuffer->Load((uint8_t*)Indices.data(), sizeof(uint32_t) * Indices.size());

	return true;
}

void FVulkanMesh::Unload()
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
