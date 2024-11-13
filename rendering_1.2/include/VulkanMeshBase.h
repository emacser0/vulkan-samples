#pragma once

#include "VulkanObject.h"
#include "VulkanBuffer.h"

class FVulkanMeshBase : public FVulkanObject
{
public:
	FVulkanMeshBase(class FVulkanContext* InContext);

	virtual bool Load(class FMesh* InMesh) { return false; }
	virtual void Unload() { }

	FVulkanBuffer* GetVertexBuffer() const { return VertexBuffer; }
	FVulkanBuffer* GetIndexBuffer() const { return IndexBuffer; }

	class FMesh* GetMeshAsset() const { return MeshAsset; }
	
protected:
	class FMesh* MeshAsset;

	FVulkanBuffer* VertexBuffer;
	FVulkanBuffer* IndexBuffer;
};
