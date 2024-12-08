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
#include "VulkanPipeline.h"
#include "VulkanSampler.h"

#include "Vertex.h"

#include <vector>
#include <unordered_map>

class FVulkanSkyRenderer : public FVulkanObject
{
public:
	FVulkanSkyRenderer(FVulkanContext* InContext);
	virtual ~FVulkanSkyRenderer();

	void PreRender();
	void Render();

protected:
	void CreateGraphicsPipelines();
	void CreateTextureSampler();
	void CreateDescriptorSetLayout();
	void CreateDescriptorSets();
	void CreateUniformBuffers();

	void GetVertexInputBindings(std::vector<VkVertexInputBindingDescription>& OutDescs);
	void GetVertexInputAttributes(std::vector<VkVertexInputAttributeDescription>& OutDescs);

	void UpdateDescriptorSets();
	void UpdateUniformBuffer();

	class FVulkanMesh* GetSkyMesh() const;

protected:
	FVulkanPipeline* Pipeline;

	VkDescriptorSetLayout DescriptorSetLayout;
	std::vector<VkDescriptorSet> DescriptorSets;

	std::vector<FVulkanBuffer*> UniformBuffers;

	FVulkanSampler* Sampler;

	bool bInitialized;
};

