#pragma once

#include "vulkan/vulkan.h"
#include "glfw/glfw3.h"

#include "glm/glm.hpp"

#include "VulkanObject.h"
#include "VulkanContext.h"
#include "VulkanBuffer.h"
#include "VulkanMesh.h"
#include "VulkanShader.h"
#include "VulkanModel.h"

#include "Vertex.h"

#include <vector>
#include <unordered_map>

#define MAX_CONCURRENT_FRAME 2

struct FVulkanPipeline
{
	VkPipeline Pipeline;
	VkPipelineLayout Layout;
	FVulkanShader* VertexShader;
	FVulkanShader* FragmentShader;
};

class FVulkanMeshRenderer : public FVulkanObject
{
public:
	FVulkanMeshRenderer(FVulkanContext* InContext);
	virtual ~FVulkanMeshRenderer();

	void Ready();
	void Render();

	void WaitIdle();

	void SetPipelineIndex(int32_t Idx);

protected:
	void CreateGraphicsPipelines();
	void CreateTextureSampler();
	void CreateUniformBuffers();
	void CreateDescriptorSetLayout();
	void CreateDescriptorSets();

	void UpdateUniformBuffer(FVulkanModel* InModel);
	void UpdateDescriptorSets();

protected:
	std::vector<FVulkanPipeline> Pipelines;
	int32_t CurrentPipelineIndex = 0;

	VkDescriptorSetLayout DescriptorSetLayout;

	std::unordered_map<FVulkanModel*, std::vector<VkDescriptorSet>> DescriptorSetMap;
	std::unordered_map<FVulkanModel*, std::vector<FVulkanBuffer>> UniformBufferMap;

	VkSampler TextureSampler;
};

