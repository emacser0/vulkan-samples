#pragma once

#include "VulkanObject.h"
#include "VulkanBuffer.h"
#include "VulkanMesh.h"
#include "VulkanShader.h"
#include "VulkanModel.h"
#include "VulkanPipeline.h"
#include "VulkanSampler.h"
#include "VulkanScene.h"

#include "vulkan/vulkan.h"

#include <vector>
#include <unordered_map>

class FVulkanRenderer : public FVulkanObject
{
public:
	FVulkanRenderer(class FVulkanContext* InContext);
	virtual ~FVulkanRenderer();

	void SetScene(class FVulkanScene* InScene) { Scene = InScene; }

	virtual void Render() = 0;
	virtual void OnRecreateSwapchain() { }

protected:
	class FVulkanScene* Scene;
};
