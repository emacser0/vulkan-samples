#include "VulkanRenderPass.h"
#include "VulkanFramebuffer.h"
#include "VulkanContext.h"
#include "VulkanHelpers.h"

FVulkanRenderPass::FVulkanRenderPass(FVulkanContext* InContext)
	: FVulkanObject(InContext)
	, RenderPass(VK_NULL_HANDLE)
{

}

FVulkanRenderPass* FVulkanRenderPass::Create(
	class FVulkanContext* InContext,
	const VkRenderPassCreateInfo& RenderPassCI)
{
	FVulkanRenderPass* RenderPass = InContext->CreateObject<FVulkanRenderPass>();

	VkDevice Device = InContext->GetDevice();
	VK_ASSERT(vkCreateRenderPass(Device, &RenderPassCI, nullptr, &RenderPass->RenderPass));

	return RenderPass;
}

void FVulkanRenderPass::Destroy()
{
	VkDevice Device = Context->GetDevice();
	vkDestroyRenderPass(Device, RenderPass, nullptr);
}

void FVulkanRenderPass::Begin(
	VkCommandBuffer InCommandBuffer,
	FVulkanFramebuffer* InFramebuffer,
	VkRect2D InRenderArea,
	const std::vector<VkClearValue>& InClearValues)
{
	VkRenderPassBeginInfo RenderPassBeginInfo{};
	RenderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	RenderPassBeginInfo.renderPass = RenderPass;
	RenderPassBeginInfo.framebuffer = InFramebuffer->GetHandle();
	RenderPassBeginInfo.renderArea = InRenderArea;
	RenderPassBeginInfo.clearValueCount = static_cast<uint32_t>(InClearValues.size());
	RenderPassBeginInfo.pClearValues = InClearValues.data();

	vkCmdBeginRenderPass(InCommandBuffer, &RenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void FVulkanRenderPass::End(VkCommandBuffer InCommandBuffer)
{
	vkCmdEndRenderPass(InCommandBuffer);
}

