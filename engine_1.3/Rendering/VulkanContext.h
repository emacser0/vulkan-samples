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
	class FVulkanSwapchain* GetSwapchain() const { return Swapchain; }
	class FVulkanViewport* GetViewport() const { return Viewport; }
	VkCommandPool GetCommandPool() const { return CommandPool; }
	const std::vector<VkCommandBuffer>& GetCommandBuffers() const { return CommandBuffers; }
	VkCommandBuffer GetCommandBuffer() const { return CommandBuffers[CurrentFrame]; }
	VkDescriptorPool GetDescriptorPool() const { return DescriptorPool; }
	uint32_t GetCurrentFrame() const { return CurrentFrame; }
	uint32_t GetMaxConcurrentFrames() const { return MAX_CONCURRENT_FRAME; }

	bool IsFramebufferResized() const { return bFramebufferResized; }
	void SetFramebufferResized(bool InbFramebufferResized) { bFramebufferResized = InbFramebufferResized; }

	class FVulkanMeshRenderer* GetMeshRenderer() const { return MeshRenderer; }
	class FVulkanUIRenderer* GetUIRenderer() const { return UIRenderer; }

	void WaitIdle();

	template<typename T = FVulkanObject>
	T* CreateObject()
	{
		T* NewObject = new T(this);
		LiveObjects.push_back(static_cast<FVulkanObject*>(NewObject));
		return NewObject;
	}
	void DestroyObject(FVulkanObject* InObject);
	bool IsValidObject(FVulkanObject* InObject);

public:
	void Render();
	void BeginRender();
	void EndRender();

protected:
	void CreateInstance();
	void SetupDebugMessenger();
	void CreateSurface();
	void PickPhysicalDevice();
	void CreateLogicalDevice();
	void CreateCommandPool();
	void CreateCommandBuffers();
	void CreateSyncObjects();
	void CreateDescriptorPool();
	void CreateViewport();
	void CreateRenderers();

	void CreateSwapchain();
	void CreateFramebuffers();

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

	class FVulkanSwapchain* Swapchain;
	std::vector<class FVulkanFramebuffer*> SwapchainFramebuffers;
	class FVulkanViewport* Viewport;

	class FVulkanImage* DepthImage;

	class FVulkanRenderer* SkyRenderer;
	class FVulkanRenderer* ShadowRenderer;
	class FVulkanMeshRenderer* MeshRenderer;
	class FVulkanUIRenderer* UIRenderer;
	std::vector<class FVulkanRenderer*> Renderers;

	VkCommandPool CommandPool;

	std::vector<VkCommandBuffer> CommandBuffers;

	VkDescriptorPool DescriptorPool;

	std::vector<VkSemaphore> ImageAcquiredSemaphores;
	std::vector<VkSemaphore> RenderFinishedSemaphores;
	std::vector<VkFence> Fences;

	uint32_t CurrentFrame;

	bool bFramebufferResized = false;

	std::vector<FVulkanObject*> LiveObjects;
};
