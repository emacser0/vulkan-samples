#pragma once

#include "VulkanObject.h"

#include "vulkan/vulkan.h"

#include <vector>

class FVulkanFramebuffer : public FVulkanObject
{
public:
	static FVulkanFramebuffer* Create(
		class FVulkanContext* InContext,
		VkRenderPass InRenderPass,
		std::vector<VkImageView>& Attachments,
		VkExtent2D InSize);

	FVulkanFramebuffer(class FVulkanContext* InContext);
	virtual ~FVulkanFramebuffer() = default;

	virtual void Destroy() override;

	VkFramebuffer GetHandle() const { return Framebuffer; }

private:
	VkFramebuffer Framebuffer;
	VkRenderPass RenderPass;
	VkExtent2D Size;
};
