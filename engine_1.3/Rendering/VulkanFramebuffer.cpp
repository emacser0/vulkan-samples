#include "VulkanFramebuffer.h"
#include "VulkanContext.h"
#include "VulkanHelpers.h"

FVulkanFramebuffer* FVulkanFramebuffer::Create(
	FVulkanContext* InContext,
	VkRenderPass InRenderPass,
	std::vector<VkImageView>& Attachments,
	VkExtent2D InSize)
{
	FVulkanFramebuffer* Framebuffer = InContext->CreateObject<FVulkanFramebuffer>();
	Framebuffer->RenderPass = InRenderPass;
	Framebuffer->Size = InSize;

	VkFramebufferCreateInfo FramebufferCI{};
	FramebufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	FramebufferCI.renderPass = InRenderPass;
	FramebufferCI.attachmentCount = static_cast<uint32_t>(Attachments.size());
	FramebufferCI.pAttachments = Attachments.data();
	FramebufferCI.width = InSize.width;
	FramebufferCI.height = InSize.height;
	FramebufferCI.layers = 1;
	FramebufferCI.pNext = nullptr;

	VkDevice Device = InContext->GetDevice();
	VK_ASSERT(vkCreateFramebuffer(Device, &FramebufferCI, nullptr, &Framebuffer->Framebuffer));

	return Framebuffer;
}

FVulkanFramebuffer::FVulkanFramebuffer(FVulkanContext* InContext)
	: FVulkanObject(InContext)
	, Framebuffer(nullptr)
	, RenderPass(VK_NULL_HANDLE)
	, Size({})
{
}

void FVulkanFramebuffer::Destroy()
{
	VkDevice Device = Context->GetDevice();
	if (Framebuffer != VK_NULL_HANDLE)
	{
		vkDestroyFramebuffer(Device, Framebuffer, nullptr);
	}
}
