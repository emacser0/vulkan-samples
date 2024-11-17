#pragma once

#include "vulkan/vulkan.h"
#include "glfw/glfw3.h"

#include "glm/glm.hpp"

#include "VulkanObject.h"
#include "VulkanContext.h"
#include "VulkanBuffer.h"
#include "VulkanMeshBase.h"
#include "VulkanShader.h"
#include "VulkanModel.h"
#include "VulkanPipeline.h"
#include "VulkanSampler.h"

#include "Vertex.h"

#include <vector>
#include <unordered_map>

#define MAX_CONCURRENT_FRAME 2

class FVulkanMeshRenderer : public FVulkanObject
{
public:
	FVulkanMeshRenderer(FVulkanContext* InContext);
	virtual ~FVulkanMeshRenderer();

	void PreRender();
	void Render();

	void SetPipelineIndex(int32_t Idx);
	void SetEnableTBNVisualization(bool bEnabled) { bTBNVisualizationEnabled = bEnabled; }

protected:
	void GenerateInstancedDrawingInfo();

	void CreateGraphicsPipelines();
	void CreateTBNPipeline();
	void CreateTextureSampler();
	void CreateUniformBuffers();
	void CreateInstanceBuffers();
	void CreateDescriptorSetLayout();
	void CreateDescriptorSets();

	void GetVertexInputBindings(std::vector<VkVertexInputBindingDescription>& OutDescs);
	void GetVertexInputAttributes(std::vector<VkVertexInputAttributeDescription>& OutDescs);

	void UpdateUniformBuffer();
	void UpdateInstanceBuffer(FVulkanMeshBase* InMesh);
	void UpdateDescriptorSets();

	void Draw(FVulkanPipeline* InPipeline, VkViewport& InViewport, VkRect2D& InScissor);

protected:
	std::vector<FVulkanPipeline*> Pipelines;
	int32_t CurrentPipelineIndex = 0;

	FVulkanPipeline* TBNPipeline;

	VkDescriptorSetLayout DescriptorSetLayout;

	struct FInstancedDrawingInfo
	{
		std::vector<FVulkanModel*> Models;
		std::vector<FVulkanBuffer*> InstanceBuffers;
		std::vector<VkDescriptorSet> DescriptorSets;
	};
	std::unordered_map<FVulkanMeshBase*, FInstancedDrawingInfo> InstancedDrawingMap;

	std::vector<FVulkanBuffer*> UniformBuffers;

	FVulkanSampler* Sampler;

	bool bInitialized;
	bool bTBNVisualizationEnabled;
};

