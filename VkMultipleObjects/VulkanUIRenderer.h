#pragma once

#include "vulkan/vulkan.h"
#include "glm/glm.hpp"

#include "VulkanTexture.h"
#include "VulkanBuffer.h"

#define MAX_CONCURRENT_FRAME 2

class FVulkanUIRenderer
{
private:
	struct FPushConstantsBlock
	{
		glm::vec2 Scale;
		glm::vec2 Translate;
	};

public:
	FVulkanUIRenderer(class FVulkanContext* InContext);
	virtual ~FVulkanUIRenderer();

	void Ready();
	void Render();

private:
	FVulkanContext* Context;
};
