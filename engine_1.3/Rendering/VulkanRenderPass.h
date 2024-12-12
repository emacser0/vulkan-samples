#pragma once

#include "VulkanObject.h"

#include "vulkan/vulkan.h"

#include <vector>

struct FVulkanRenderTargetLayout
{
public:
	VkOffset2D Offset;
	VkExtent2D Extent;
};

class FVulkanRenderPass : public FVulkanObject
{
public:
	FVulkanRenderPass(class FVulkanContext* InContext);
	virtual ~FVulkanRenderPass() = default;

	static FVulkanRenderPass* Create(
		class FVulkanContext* InContext,
		const VkRenderPassCreateInfo& RenderPassCI);

	virtual void Destroy() override;

	VkRenderPass GetHandle() const { return RenderPass; }

	void Begin(
		VkCommandBuffer InCommandBuffer,
		class FVulkanFramebuffer* InFramebuffer,
		VkRect2D InRenderArea,
		const std::vector<VkClearValue>& InClearValues);
	void End(VkCommandBuffer InCommandBuffer);

private:
	VkRenderPass RenderPass;
};
