#pragma once

#include "VulkanObject.h"
#include "VulkanBuffer.h"

#include "Mesh.h"
#include "Vertex.h"

#include "vulkan/vulkan.h"

#include <cstdint>
#include <string>

class FVulkanMesh : public FVulkanObject
{
public:
	FVulkanMesh(class FVulkanContext* InContext);
	virtual ~FVulkanMesh();

	bool Load(FMesh* InMesh);
	void Unload();

	FVulkanBuffer GetVertexBuffer() const { return VertexBuffer; }
	FVulkanBuffer GetIndexBuffer() const { return IndexBuffer; }

	uint32_t GetNumVertices() const { return NumVertices; }
	uint32_t GetNumIndices() const { return NumIndices; }

	class FVulkanTexture* GetTexture() const { return Texture; }
	void SetTexture(class FVulkanTexture* InTexture) { Texture = InTexture; }

private:
	void CreateVertexBuffer(const std::vector<FVertex>& Vertices);
	void CreateIndexBuffer(const std::vector<uint32_t>& Indices);
	
private:
	FVulkanBuffer VertexBuffer;
	FVulkanBuffer IndexBuffer;

	class FVulkanTexture* Texture;

	uint32_t NumVertices;
	uint32_t NumIndices;

	bool bLoaded;
};
