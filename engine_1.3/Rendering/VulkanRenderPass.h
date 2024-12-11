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

	virtual void Destroy() override;

	void Begin(VkCommandBuffer InCommandBuffer);
	void End(VkCommandBuffer InCommandBuffer);

private:
	VkRenderPass RenderPass;
};
