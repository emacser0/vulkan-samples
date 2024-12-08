#pragma once

#include "VulkanObject.h"
#include "VulkanBuffer.h"
#include "VulkanMaterial.h"

class FVulkanMesh : public FVulkanObject
{
public:
	FVulkanMesh(class FVulkanContext* InContext);

	virtual void Destroy() override;

	virtual bool Load(class UMesh* InMesh);
	virtual void Unload();

	FVulkanBuffer* GetVertexBuffer() const { return VertexBuffer; }
	FVulkanBuffer* GetIndexBuffer() const { return IndexBuffer; }

	FVulkanMaterial* GetMaterial() const { return Material; }
	void SetMaterial(FVulkanMaterial* InMaterial) { Material = InMaterial; }

	class UMesh* GetMeshAsset() const { return MeshAsset; }
	
protected:
	FVulkanBuffer* VertexBuffer;
	FVulkanBuffer* IndexBuffer;
	FVulkanMaterial* Material;

	class UMesh* MeshAsset;
};

