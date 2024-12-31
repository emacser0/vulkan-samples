#pragma once

#include "VulkanObject.h"

#include "Vertex.h"

#include "vulkan/vulkan.h"
#include "glfw/glfw3.h"
#include "glm/glm.hpp"

#include <vector>
#include <unordered_map>
#include <memory>

#define MAX_CONCURRENT_FRAME 2

class FVulkanMeshRenderer : public FVulkanObject
{
public:
	FVulkanMeshRenderer(FVulkanContext* InContext);
	virtual ~FVulkanMeshRenderer();

	void Render();

	void SetViewMatrix(const glm::mat4& InViewMatrix) { ViewMatrix = InViewMatrix; }
	void SetProjectionMatrix(const glm::mat4& InProjectionMatrix) { ProjectionMatrix = InProjectionMatrix; }

protected:
	void CreateGraphicsPipeline();
	void CreateTextureSampler();
	void CreateUniformBuffers();
	void CreateDescriptorSetLayout();
	void CreateDescriptorSets();

	void UpdateUniformBuffer(class FVulkanModel* InModel);
	void UpdateDescriptorSets();

protected:
	VkPipelineLayout PipelineLayout;
	VkPipeline Pipeline;

	VkDescriptorSetLayout DescriptorSetLayout;

	std::unordered_map<class FVulkanModel*, std::vector<VkDescriptorSet>> DescriptorSetMap;
	std::unordered_map<class FVulkanModel*, std::vector<struct FVulkanBuffer>> UniformBufferMap;

	glm::mat4 ViewMatrix;
	glm::mat4 ProjectionMatrix;

	VkSampler TextureSampler;

	std::shared_ptr<class FVulkanShader> VertexShader;
	std::shared_ptr<class FVulkanShader> FragmentShader;
};

