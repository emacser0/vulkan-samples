#pragma once

#include "vulkan/vulkan.h"
#include "glfw/glfw3.h"

#include <vector>

#include "VulkanObject.h"

#define MAX_CONCURRENT_FRAME 2

class FVulkanContext
{
public:
	FVulkanContext(GLFWwindow* InWindow);
	virtual ~FVulkanContext();

	GLFWwindow* GetWindow() const { return Window; }
	VkInstance GetInstance() const { return Instance; }
	VkSurfaceKHR GetSurface() const { return Surface; }
	VkPhysicalDevice GetPhysicalDevice() const { return PhysicalDevice; }
	VkDevice GetDevice() const { return Device; }
	VkQueue GetGfxQueue() const { return GfxQueue; }
	VkQueue GetPresentQueue() const { return PresentQueue; }
	VkSwapchainKHR GetSwapchain() const { return Swapchain; }
	VkFormat GetSwapchainImageFormat() const { return SwapchainImageFormat; }
	VkExtent2D GetSwapchainExtent() const { return SwapchainExtent; }
	VkRenderPass GetRenderPass() const { return RenderPass; }
	VkCommandPool GetCommandPool() const { return CommandPool; }
	const std::vector<VkCommandBuffer>& GetCommandBuffers() const { return CommandBuffers; }
	VkDescriptorPool GetDescriptorPool() const { return DescriptorPool; }
	uint32_t GetCurrentFrame() const { return CurrentFrame; }
	uint32_t GetMaxConcurrentFrames() const { return MAX_CONCURRENT_FRAME; }
	uint32_t GetCurrentImageIndex() const { return CurrentImageIndex; }

	bool IsFramebufferResized() const { return bFramebufferResized; }
	void SetFramebufferResized(bool InbFramebufferResized) { bFramebufferResized = InbFramebufferResized; }

	void WaitIdle();

	template<typename T = FVulkanObject>
	T* CreateObject()
	{
		T* NewObject = new T(this);
		LiveObjects.push_back(static_cast<FVulkanObject*>(NewObject));
		return NewObject;
	}

	void DestroyObject(FVulkanObject* InObject);

public:

	void BeginRender();
	void EndRender();

protected:
	void CreateInstance();
	void SetupDebugMessenger();
	void CreateSurface();
	void PickPhysicalDevice();
	void CreateLogicalDevice();
	void CreateRenderPass();
	void CreateCommandPool();
	void CreateCommandBuffers();
	void CreateSyncObjects();
	void CreateDescriptorPool();

	void CreateSwapchain();
	void CreateImageViews();
	void CreateFramebuffers();
	void CreateDepthResources();

	void CleanupSwapchain();
	void RecreateSwapchain();

protected:
	GLFWwindow* Window;

	VkInstance Instance;
	VkDebugUtilsMessengerEXT DebugMessenger;
	VkSurfaceKHR Surface;

	VkPhysicalDevice PhysicalDevice;
	VkDevice Device;

	VkQueue GfxQueue;
	VkQueue PresentQueue;

	VkSwapchainKHR Swapchain;
	std::vector<VkImage> SwapchainImages;
	VkFormat SwapchainImageFormat;
	VkExtent2D SwapchainExtent;
	std::vector<VkImageView> SwapchainImageViews;
	std::vector<VkFramebuffer> SwapchainFramebuffers;

	class FVulkanImage* DepthImage;

	VkRenderPass RenderPass;
	VkCommandPool CommandPool;

	std::vector<VkCommandBuffer> CommandBuffers;

	VkDescriptorPool DescriptorPool;

	std::vector<VkSemaphore> ImageAvailableSemaphores;
	std::vector<VkSemaphore> RenderFinishedSemaphores;
	std::vector<VkFence> Fences;

	uint32_t CurrentFrame;
	uint32_t CurrentImageIndex;

	bool bFramebufferResized = false;

	std::vector<FVulkanObject*> LiveObjects;
};
