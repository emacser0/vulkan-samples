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
	virtual void Destroy() override;

	void Ready();
	void Render();

	void AddWidget(const std::shared_ptr<class FWidget>& InWidget);
	void RemoveWidget(const std::shared_ptr<class FWidget>& InWidget);

private:
	std::vector<std::shared_ptr<class FWidget>> Widgets;
};
