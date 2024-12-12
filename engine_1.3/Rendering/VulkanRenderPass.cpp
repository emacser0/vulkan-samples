#include "VulkanRenderPass.h"
#include "VulkanFramebuffer.h"
#include "VulkanContext.h"
#include "VulkanHelpers.h"
#include "VulkanSwapchain.h"

#include <vector>

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

FVulkanRenderPass* FVulkanRenderPass::CreateShadowPass(class FVulkanContext* InContext)
{
	return nullptr;
}

FVulkanRenderPass* FVulkanRenderPass::CreateBasePass(FVulkanContext* InContext, FVulkanSwapchain* InSwapchain)
{
	VkPhysicalDevice PhysicalDevice = InContext->GetPhysicalDevice();

	VkAttachmentDescription ColorAttachmentDesc{};
	ColorAttachmentDesc.format = InSwapchain->GetFormat();
	ColorAttachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
	ColorAttachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	ColorAttachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	ColorAttachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	ColorAttachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	ColorAttachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	ColorAttachmentDesc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentDescription DepthAttachmentDesc{};
	DepthAttachmentDesc.format = Vk::FindDepthFormat(PhysicalDevice);
	DepthAttachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
	DepthAttachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	DepthAttachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	DepthAttachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	DepthAttachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	DepthAttachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	DepthAttachmentDesc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference ColorAttachmentRef{};
	ColorAttachmentRef.attachment = 0;
	ColorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference DepthAttachmentRef{};
	DepthAttachmentRef.attachment = 1;
	DepthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription SubpassDesc{};
	SubpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	SubpassDesc.colorAttachmentCount = 1;
	SubpassDesc.pColorAttachments = &ColorAttachmentRef;
	SubpassDesc.pDepthStencilAttachment = &DepthAttachmentRef;

	VkSubpassDependency SubpassDependency{};
	SubpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	SubpassDependency.dstSubpass = 0;
	SubpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	SubpassDependency.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	SubpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	SubpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	std::vector<VkAttachmentDescription> Attachments =
	{
		ColorAttachmentDesc,
		DepthAttachmentDesc
	};

	VkRenderPassCreateInfo RenderPassCI{};
	RenderPassCI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	RenderPassCI.attachmentCount = static_cast<uint32_t>(Attachments.size());
	RenderPassCI.pAttachments = Attachments.data();
	RenderPassCI.subpassCount = 1;
	RenderPassCI.pSubpasses = &SubpassDesc;
	RenderPassCI.dependencyCount = 1;
	RenderPassCI.pDependencies = &SubpassDependency;

	return FVulkanRenderPass::Create(InContext, RenderPassCI);
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

