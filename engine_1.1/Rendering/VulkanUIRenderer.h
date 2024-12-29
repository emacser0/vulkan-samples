#pragma once

#include "vulkan/vulkan.h"
#include "glm/glm.hpp"

#include "VulkanTexture.h"
#include "VulkanBuffer.h"

#include <memory>
#include <vector>

#define MAX_CONCURRENT_FRAME 2

class FVulkanUIRenderer : public FVulkanObject
{
public:
	FVulkanUIRenderer(class FVulkanContext* InContext);
	virtual ~FVulkanUIRenderer();

	void Render(const std::vector<std::shared_ptr<class FWidget>>& InWidgets);
};
