#pragma once

#include "vulkan/vulkan.h"
#include "glfw/glfw3.h"

#include "glm/glm.hpp"

#include "VulkanObject.h"
#include "VulkanBuffer.h"
#include "VulkanMesh.h"
#include "VulkanShader.h"
#include "VulkanModel.h"

#include "Vertex.h"

#include <vector>
#include <unordered_map>

#define MAX_CONCURRENT_FRAME 2

class FVulkanMeshRenderer : public FVulkanObject
{
public:
	FVulkanMeshRenderer(FVulkanContext* InContext);
	virtual ~FVulkanMeshRenderer();

	void Ready();
	void Render();

	void WaitIdle();

protected:
	void CreateGraphicsPipeline();
	void CreateTextureSampler();
	void CreateUniformBuffers();
	void CreateDescriptorSetLayout();
	void CreateDescriptorSets();

	void UpdateUniformBuffer(FVulkanModel* InModel);
	void UpdateDescriptorSets();

protected:
	VkPipelineLayout PipelineLayout;
	VkPipeline Pipeline;

	VkDescriptorSetLayout DescriptorSetLayout;

	std::unordered_map<FVulkanModel*, std::vector<VkDescriptorSet>> DescriptorSetMap;
	std::unordered_map<FVulkanModel*, std::vector<FVulkanBuffer>> UniformBufferMap;

	VkSampler TextureSampler;

	FVulkanShader* VertexShader;
	FVulkanShader* FragmentShader;
};

