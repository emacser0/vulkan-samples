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

	void WaitIdle();

	void SetPipelineIndex(int32_t Idx);

protected:
	void GenerateInstancedDrawingInfo();

	void CreateGraphicsPipelines();
	void CreateNormalVisualizationPipeline();
	void CreateTextureSampler();
	void CreateUniformBuffers();
	void CreateInstanceBuffers();
	void CreateDescriptorSetLayout();
	void CreateDescriptorSets();

	void UpdateUniformBuffer();
	void UpdateInstanceBuffer(FVulkanMesh* InMesh);
	void UpdateDescriptorSets();

	void Draw(FVulkanPipeline* InPipeline);

protected:
	std::vector<FVulkanPipeline*> Pipelines;
	int32_t CurrentPipelineIndex = 0;

	FVulkanPipeline* NormalVisualizationPipeline;

	VkDescriptorSetLayout DescriptorSetLayout;

	struct FInstancedDrawingInfo
	{
		std::vector<FVulkanModel*> Models;
		std::vector<FVulkanBuffer> InstanceBuffers;
		std::vector<VkDescriptorSet> DescriptorSets;
	};
	std::unordered_map<FVulkanMesh*, FInstancedDrawingInfo> InstancedDrawingMap;

	std::vector<FVulkanBuffer> UniformBuffers;

	VkSampler TextureSampler;

	bool bInitialized;
};

