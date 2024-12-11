#include "VulkanRenderPass.h"
#include "VulkanContext.h"

FVulkanRenderPass::FVulkanRenderPass(FVulkanContext* InContext)
	: FVulkanObject(InContext)
	, RenderPass(VK_NULL_HANDLE)
{

}

void FVulkanRenderPass::Destroy()
{

}