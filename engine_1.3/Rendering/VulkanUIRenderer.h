#pragma once

#include "VulkanRenderer.h"
#include "VulkanTexture.h"
#include "VulkanBuffer.h"

#include "vulkan/vulkan.h"
#include "glm/glm.hpp"

#include <memory>
#include <vector>

#define MAX_CONCURRENT_FRAME 2

class FVulkanUIRenderer : public FVulkanRenderer
{
public:
	FVulkanUIRenderer(class FVulkanContext* InContext);
	virtual void Destroy() override;

	void Render();

	void AddWidget(const std::shared_ptr<class FWidget>& InWidget);
	void RemoveWidget(const std::shared_ptr<class FWidget>& InWidget);

private:
	std::vector<std::shared_ptr<class FWidget>> Widgets;
};
