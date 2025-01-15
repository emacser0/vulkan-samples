#pragma once

#include "VulkanRenderer.h"
#include "VulkanBuffer.h"

#include "vulkan/vulkan.h"
#include "glfw/glfw3.h"
#include "glm/glm.hpp"

#include "Vertex.h"

#include <vector>
#include <unordered_map>

class FVulkanMeshRenderer : public FVulkanRenderer
{
public:
	FVulkanMeshRenderer(class FVulkanContext* InContext);
	virtual ~FVulkanMeshRenderer();

	virtual void Destroy() override;

	virtual void Render() override;
	virtual void OnRecreateSwapchain() override;

	void SetEnableTBNVisualization(bool bEnabled) { bEnableTBNVisualization = bEnabled; }
	void SetEnableAttenuation(bool bEnabled) { bEnableAttenuation = bEnabled; }
	void SetEnableGammaCorrection(bool bEnabled) { bEnableGammaCorrection = bEnabled; }
	void SetEnableToneMapping(bool bEnabled) { bEnableToneMapping = bEnabled; }

protected:
	void GenerateInstancedDrawingInfo();

	void CreateRenderPasses();
	void CreateShadowDepthImage();
	void CreateFramebuffers();
	void CreateDescriptorSetLayout();
	void CreateGraphicsPipelines();
	void CreateShadowPipeline();
	void CreateTBNPipeline();
	void CreateTextureSampler();
	void CreateUniformBuffers();
	void CreateInstanceBuffers();
	void CreateDescriptorSets();

	void GetVertexInputBindings(std::vector<VkVertexInputBindingDescription>& OutDescs);
	void GetVertexInputAttributes(std::vector<VkVertexInputAttributeDescription>& OutDescs);

	void UpdateUniformBuffer(bool bIsShadowPass);
	void UpdateMaterialBuffer(class FVulkanMesh* InMesh);
	void UpdateInstanceBuffer(class FVulkanMesh* InMesh);
	void UpdateDescriptorSets();

	struct FInstancedDrawingInfo
	{
		class FVulkanPipeline* Pipeline;
		std::vector<class FVulkanModel*> Models;
		std::vector<class FVulkanBuffer*> InstanceBuffers;
		std::vector<VkDescriptorSet> DescriptorSets;
	};
	void Draw(class FVulkanMesh* InMesh, const FInstancedDrawingInfo& InDrawingInfo, VkViewport& InViewport, VkRect2D& InScissor);

protected:
	class FVulkanRenderPass* ShadowPass;
	class FVulkanRenderPass* BasePass;

	class FVulkanImage* ShadowDepthImage;

	std::vector<class FVulkanFramebuffer*> ShadowFramebuffers;
	std::vector<class FVulkanFramebuffer*> Framebuffers;

	class FVulkanPipeline* ShadowPipeline;
	class FVulkanPipeline* TBNPipeline;

	VkDescriptorSetLayout DescriptorSetLayout;

	std::unordered_map<class FVulkanMesh*, FInstancedDrawingInfo> InstancedDrawingMap;

	std::vector<class FVulkanBuffer*> TransformBuffers;
	std::vector<class FVulkanBuffer*> LightBuffers;
	std::vector<class FVulkanBuffer*> MaterialBuffers;
	std::vector<class FVulkanBuffer*> DebugBuffers;

	class FVulkanSampler* Sampler;

	bool bInitialized;
	bool bEnableTBNVisualization;
	bool bEnableAttenuation;
	bool bEnableGammaCorrection;
	bool bEnableToneMapping;
};

