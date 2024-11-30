#pragma once

#include "VulkanObject.h"
#include "VulkanBuffer.h"

class FVulkanMeshBase : public FVulkanObject
{
public:
	FVulkanMeshBase(class FVulkanContext* InContext);

	virtual void Destroy() override;

	virtual bool Load(class UMesh* InMesh);
	virtual void Unload();

	FVulkanBuffer* GetVertexBuffer() const { return VertexBuffer; }
	FVulkanBuffer* GetIndexBuffer() const { return IndexBuffer; }

	class UMesh* GetMeshAsset() const { return MeshAsset; }
	
protected:
	class UMesh* MeshAsset;

	FVulkanBuffer* VertexBuffer;
	FVulkanBuffer* IndexBuffer;
};
