#pragma once

#include "vulkan/vulkan.h"
#include "glfw/glfw3.h"

#include "glm/glm.hpp"

#include "Math.h"
#include "VulkanBuffer.h"
#include "VulkanTexture.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <set>
#include <stdexcept>
#include <algorithm>
#include <limits>
#include <cstddef>
#include <cassert>

#define MAX_CONCURRENT_FRAME 2

class FVulkanMeshRenderer
{
public:
	FVulkanMeshRenderer(class FVulkanContext* InContext);
	virtual ~FVulkanMeshRenderer();

	std::vector<FVertex>& GetVertices() { return Vertices; }
	std::vector<uint32_t>& GetIndices() { return Indices;  }
	FVulkanTexture& GetTexture() { return Texture; }

	void Ready();
	void Render();

	void WaitIdle();

	void SetViewMatrix(const glm::mat4& InViewMatrix) { ViewMatrix = InViewMatrix; }
	void SetProjectionMatrix(const glm::mat4& InProjectionMatrix) { ProjectionMatrix = InProjectionMatrix; }

protected:
	void RecordCommandBuffer(VkCommandBuffer InCommandBuffer, uint32_t InCurrentFrame, uint32_t InImageIndex);

	void CreateGraphicsPipeline();
	void CreateTextureImage();
	void CreateTextureImageView();
	void CreateTextureSampler();
	void CreateVertexBuffer();
	void CreateIndexBuffer();
	void CreateUniformBuffers();
	void CreateDescriptorSetLayout();
	void CreateDescriptorSets();

	void UpdateUniformBuffer();

protected:
	class FVulkanContext* Context;

	VkPipelineLayout PipelineLayout;
	VkPipeline Pipeline;

	VkDescriptorSetLayout DescriptorSetLayout;
	std::vector<VkDescriptorSet> DescriptorSets;

	FVulkanTexture Texture;
	VkSampler TextureSampler;

	FVulkanBuffer VertexBuffer;
	FVulkanBuffer IndexBuffer;

	std::vector<FVulkanBuffer> UniformBuffers;
	glm::mat4 ViewMatrix;
	glm::mat4 ProjectionMatrix;

	std::vector<FVertex> Vertices;
	std::vector<uint32_t> Indices;
};

