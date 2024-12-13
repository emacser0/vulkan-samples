#pragma once

#include "VulkanObject.h"

#include "vulkan/vulkan.h"
#include "glfw/glfw3.h"

#include <vector>

class FVulkanViewport : public FVulkanObject
{
public:
	FVulkanViewport(class FVulkanContext* InContext);
	virtual ~FVulkanViewport();

	static FVulkanViewport* Create(FVulkanContext* InContext, GLFWwindow* InWindow);

	virtual void Destroy() override;

	void CreateSwapchain(GLFWwindow* InWindow);
	void CreateDepthImage();

	void Recreate();
	void Cleanup();

	class FVulkanSwapchain* GetSwapchain() const { return Swapchain; }
	class FVulkanImage* GetDepthImage() const { return DepthImage; }

private:
	class FVulkanSwapchain* Swapchain;
	class FVulkanImage* DepthImage;
};
