#pragma once

#include "VulkanObject.h"
#include "VulkanBuffer.h"

#include "Vertex.h"

#include "vulkan/vulkan.h"

#include <cstdint>
#include <string>

class FVulkanMesh : public FVulkanObject
{
public:
	FVulkanMesh(class FVulkanContext* InContext);
	virtual ~FVulkanMesh();

	bool Load(class FMesh* InMesh);
	void Unload();

	FVulkanBuffer GetVertexBuffer() const { return VertexBuffer; }
	FVulkanBuffer GetIndexBuffer() const { return IndexBuffer; }

	class FMesh* GetMeshAsset() const { return MeshAsset; }

	class FVulkanTexture* GetBaseColorTexture() const { return BaseColorTexture; }
	void SetBaseColorTexture(class FVulkanTexture* InTexture) { BaseColorTexture = InTexture; }

	class FVulkanTexture* GetNormalTexture() const { return NormalTexture; }
	void SetNormalTexture(class FVulkanTexture* InTexture) { NormalTexture = InTexture; }

private:
	void CreateVertexBuffer(const std::vector<FVertex>& Vertices);
	void CreateIndexBuffer(const std::vector<uint32_t>& Indices);
	
private:
	class FMesh* MeshAsset;

	FVulkanBuffer VertexBuffer;
	FVulkanBuffer IndexBuffer;

	class FVulkanTexture* BaseColorTexture;
	class FVulkanTexture* NormalTexture;

	bool bLoaded;
};
