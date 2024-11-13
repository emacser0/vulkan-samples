#include "VulkanMeshBase.h"
#include "VulkanContext.h"

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
