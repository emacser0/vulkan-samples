#include "VulkanPipeline.h"
#include "VulkanContext.h"
#include "VulkanHelpers.h"

FVulkanPipeline::FVulkanPipeline(FVulkanContext* InContext)
	: FVulkanObject(InContext)
	, Pipeline(VK_NULL_HANDLE)
	, Layout(VK_NULL_HANDLE)
	, VS(VK_NULL_HANDLE)
	, FS(VK_NULL_HANDLE)
{

}

FVulkanPipeline::~FVulkanPipeline()
{

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
