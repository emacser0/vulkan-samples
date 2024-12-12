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

class FVulkanMeshRenderer : public FVulkanObject
{
public:
	FVulkanMeshRenderer(class FVulkanContext* InContext);
	virtual ~FVulkanMeshRenderer();

	void PreRender();
	void Render();

	void SetScene(class FVulkanScene* InScene) { Scene = InScene; }

	void SetEnableTBNVisualization(bool bEnabled) { bEnableTBNVisualization = bEnabled; }
	void SetEnableAttenuation(bool bEnabled) { bEnableAttenuation = bEnabled; }
	void SetEnableGammaCorrection(bool bEnabled) { bEnableGammaCorrection = bEnabled; }
	void SetEnableToneMapping(bool bEnabled) { bEnableToneMapping = bEnabled; }

protected:
	void GenerateInstancedDrawingInfo();

	void CreateRenderPass();
	void CreateDescriptorSetLayout();
	void CreateGraphicsPipelines();
	void CreateTBNPipeline();
	void CreateTextureSampler();
	void CreateUniformBuffers();
	void CreateInstanceBuffers();
	void CreateDescriptorSets();

	void GetVertexInputBindings(std::vector<VkVertexInputBindingDescription>& OutDescs);
	void GetVertexInputAttributes(std::vector<VkVertexInputAttributeDescription>& OutDescs);

	void UpdateUniformBuffer();
	void UpdateMaterialBuffer(FVulkanMesh* InMesh);
	void UpdateInstanceBuffer(FVulkanMesh* InMesh);
	void UpdateDescriptorSets();

	struct FInstancedDrawingInfo
	{
		class FVulkanPipeline* Pipeline;
		std::vector<FVulkanModel*> Models;
		std::vector<FVulkanBuffer*> InstanceBuffers;
		std::vector<VkDescriptorSet> DescriptorSets;
	};
	void Draw(FVulkanMesh* InMesh, const FInstancedDrawingInfo& InDrawingInfo, VkViewport& InViewport, VkRect2D& InScissor);

protected:
	class FVulkanScene* Scene;
	class FVulkanRenderPass* RenderPass;

	class FVulkanPipeline* TBNPipeline;

	VkDescriptorSetLayout DescriptorSetLayout;

	std::unordered_map<FVulkanMesh*, FInstancedDrawingInfo> InstancedDrawingMap;

	std::vector<FVulkanBuffer*> TransformBuffers;
	std::vector<FVulkanBuffer*> LightBuffers;
	std::vector<FVulkanBuffer*> MaterialBuffers;
	std::vector<FVulkanBuffer*> DebugBuffers;

	class FVulkanSampler* Sampler;

	bool bInitialized;
	bool bEnableTBNVisualization;
	bool bEnableAttenuation;
	bool bEnableGammaCorrection;
	bool bEnableToneMapping;
};

