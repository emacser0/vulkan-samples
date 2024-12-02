#include "VulkanMesh.h"
#include "VulkanContext.h"

#include "Vertex.h"
#include "Mesh.h"
#include "Material.h"

#include <vector>

FVulkanMesh::FVulkanMesh(FVulkanContext* InContext)
	: FVulkanObject(InContext)
	, MeshAsset(nullptr)
	, VertexBuffer(nullptr)
	, IndexBuffer(nullptr)
{
	VertexBuffer = InContext->CreateObject<FVulkanBuffer>();
	VertexBuffer->SetUsage(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
	VertexBuffer->SetProperties(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	IndexBuffer = InContext->CreateObject<FVulkanBuffer>();
	IndexBuffer->SetUsage(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
	IndexBuffer->SetProperties(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	Material = InContext->CreateObject<FVulkanMaterial>();
}

void FVulkanMesh::Destroy()
{
	Unload();
}

bool FVulkanMesh::Load(UMesh* InMesh)
{
	if (InMesh == nullptr)
	{
		return false;
	}

	UMaterial* MaterialAsset = InMesh->GetMaterial();
	if (MaterialAsset == nullptr)
	{
		return false;
	}

	if (VertexBuffer->GetBuffer() != VK_NULL_HANDLE ||
		IndexBuffer->GetBuffer() != VK_NULL_HANDLE ||
		Material != nullptr)
	{
		Unload();
	}

	MeshAsset = InMesh;

	const std::vector<FVertex>& Vertices = InMesh->GetVertices();
	const std::vector<uint32_t>& Indices = InMesh->GetIndices();

	VertexBuffer->Load((uint8_t*)Vertices.data(), sizeof(FVertex) * Vertices.size());
	IndexBuffer->Load((uint8_t*)Indices.data(), sizeof(uint32_t) * Indices.size());

	FShaderPath ShaderPath = MaterialAsset->GetShaderPath();

	if (Material->LoadVS(ShaderPath.VS) == false)
	{
		Unload();
		return false;
	}

	if (Material->LoadFS(ShaderPath.FS) == false)
	{
		Unload();
		return false;
	}

	Material->SetBaseColor(MaterialAsset->GetBaseColor());
	Material->SetNormal(MaterialAsset->GetNormal());

	return true;
}

void FVulkanMesh::Unload()
{
	VkDevice Device = Context->GetDevice();

	MeshAsset = nullptr;

	if (VertexBuffer != nullptr)
	{
		VertexBuffer->Unload();
	}

	if (IndexBuffer != nullptr)
	{
		IndexBuffer->Unload();
	}

	if (Material != nullptr)
	{
		Material->UnloadShaders();
	}
}
