#include "VulkanPipeline.h"
#include "VulkanContext.h"
#include "VulkanHelpers.h"

FVulkanPipeline::FVulkanPipeline(FVulkanContext* InContext)
	: FVulkanObject(InContext)
	, Layout(VK_NULL_HANDLE)
	, Pipeline(VK_NULL_HANDLE)
	, VS(VK_NULL_HANDLE)
	, GS(VK_NULL_HANDLE)
	, FS(VK_NULL_HANDLE)
{

}

void FVulkanPipeline::Destroy()
{
	VkDevice Device = Context->GetDevice();

	if (Layout != VK_NULL_HANDLE)
	{
		vkDestroyPipelineLayout(Device, Layout, nullptr);
	}

	if (Pipeline != VK_NULL_HANDLE)
	{
		vkDestroyPipeline(Device, Pipeline, nullptr);
	}
}

void FVulkanPipeline::CreateLayout(const VkPipelineLayoutCreateInfo& CI)
{
	VkDevice Device = Context->GetDevice();

	VK_ASSERT(vkCreatePipelineLayout(Device, &CI, nullptr, &Layout));
}

void FVulkanPipeline::CreatePipeline(const VkGraphicsPipelineCreateInfo& CI)
{
	VkDevice Device = Context->GetDevice();

	VK_ASSERT(vkCreateGraphicsPipelines(Device, VK_NULL_HANDLE, 1, &CI, nullptr, &Pipeline));
}
