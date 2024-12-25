#pragma once

#include "VulkanRenderer.h"

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

	void CreateFramebuffers();

	void Render();

	void AddWidget(const std::shared_ptr<class FWidget>& InWidget);
	void RemoveWidget(const std::shared_ptr<class FWidget>& InWidget);

private:
	class FVulkanRenderPass* RenderPass;
	std::vector<class FVulkanFramebuffer*> Framebuffers;

	std::vector<std::shared_ptr<class FWidget>> Widgets;
};
