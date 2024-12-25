#include "VulkanRenderer.h"
#include "VulkanContext.h"
#include "VulkanScene.h"
#include "VulkanRenderPass.h"

FVulkanRenderer::FVulkanRenderer(FVulkanContext* InContext)
	: FVulkanObject(InContext)
	, Scene(nullptr)
{

}

FVulkanRenderer::~FVulkanRenderer()
{

}
