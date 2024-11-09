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

	bool Load(const std::string& InFilename);

	const std::vector<FVertex>& GetVertices() const { return Vertices; }
	const std::vector<uint32_t>& GetIndices() const { return Indices; }

	FVulkanBuffer GetVertexBuffer() const { return VertexBuffer; }
	FVulkanBuffer GetIndexBuffer() const { return IndexBuffer; }

	class FVulkanTexture* GetTexture() const { return Texture; }
	void SetTexture(class FVulkanTexture* InTexture) { Texture = InTexture; }

private:
	void CreateVertexBuffer();
	void CreateIndexBuffer();
	
private:
	std::vector<FVertex> Vertices;
	std::vector<uint32_t> Indices;

	FVulkanBuffer VertexBuffer;
	FVulkanBuffer IndexBuffer;

	class FVulkanTexture* Texture;

	bool bLoaded;
};
