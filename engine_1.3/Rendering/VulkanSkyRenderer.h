#pragma once

#include "VulkanRenderer.h"

#include "Vertex.h"

#include "vulkan/vulkan.h"
#include "glfw/glfw3.h"
#include "glm/glm.hpp"

#include <vector>
#include <unordered_map>

class FVulkanSkyRenderer : public FVulkanRenderer
{
public:
	FVulkanSkyRenderer(class FVulkanContext* InContext);
	virtual ~FVulkanSkyRenderer();

	virtual void Destroy() override;

	virtual void Render() override;
	virtual void OnRecreateSwapchain() override;

protected:
	void CreateRenderPass();
	void CreateFramebuffers();
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
	class FVulkanRenderPass* RenderPass;
	std::vector<class FVulkanFramebuffer*> Framebuffers;

	class FVulkanPipeline* Pipeline;

	VkDescriptorSetLayout DescriptorSetLayout;
	std::vector<VkDescriptorSet> DescriptorSets;

	std::vector<class FVulkanBuffer*> UniformBuffers;

	class FVulkanSampler* Sampler;

	bool bInitialized;
};

