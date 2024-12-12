#pragma once

#include "VulkanObject.h"
#include "VulkanBuffer.h"
#include "VulkanMesh.h"
#include "VulkanShader.h"
#include "VulkanModel.h"
#include "VulkanPipeline.h"
#include "VulkanSampler.h"
#include "VulkanScene.h"

#include "vulkan/vulkan.h"

#include <vector>
#include <unordered_map>

class FVulkanRenderer : public FVulkanObject
{
public:
	FVulkanRenderer(class FVulkanContext* InContext);
	virtual ~FVulkanRenderer();

	virtual void Render(FVulkanScene* InScene) = 0;

protected:

	struct FInstancedDrawingInfo
	{
		FVulkanPipeline* Pipeline;
		std::vector<FVulkanModel*> Models;
		std::vector<FVulkanBuffer*> InstanceBuffers;
		std::vector<VkDescriptorSet> DescriptorSets;
	};
	void Draw(FVulkanMesh* InMesh, const FInstancedDrawingInfo& InDrawingInfo, VkViewport& InViewport, VkRect2D& InScissor);

protected:
	FVulkanScene* Scene;

	FVulkanPipeline* TBNPipeline;

	VkDescriptorSetLayout DescriptorSetLayout;

	std::unordered_map<FVulkanMesh*, FInstancedDrawingInfo> InstancedDrawingMap;

	std::vector<FVulkanBuffer*> TransformBuffers;
	std::vector<FVulkanBuffer*> LightBuffers;
	std::vector<FVulkanBuffer*> MaterialBuffers;
	std::vector<FVulkanBuffer*> DebugBuffers;

	FVulkanSampler* Sampler;

	bool bInitialized;
	bool bEnableTBNVisualization;
	bool bEnableAttenuation;
	bool bEnableGammaCorrection;
	bool bEnableToneMapping;
};

pragma once
