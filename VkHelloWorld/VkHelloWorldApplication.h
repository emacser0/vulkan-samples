#pragma once

#include "Application.h"

#include "vulkan/vulkan.h"
#include "glm/glm.hpp"

#include <memory>
#include <vector>

class FVkHelloWorldApplication : public FApplication
{
public:
	FVkHelloWorldApplication();

	virtual void Run() override;
	virtual void Terminate() override;

	virtual void Tick(float InDeltaTime) override;

private:
	void AddResizeCallback();
	void CreateInstance();
	void SetupDebugMessenger();
	void CreateSurface();
	void PickPhysicalDevice();
	void CreateLogicalDevice();
	void CreateSwapchain();
	void CreateImageViews();
	void CreateRenderPass();
	void CreateGraphicsPipeline();
	void CreateFramebuffers();
	void CreateCommandPool();
	void CreateVertexBuffer();
	void CreateIndexBuffer();
	void CreateCommandBuffers();
	void CreateSyncObjects();

	void RecordCommandBuffer(VkCommandBuffer InCommandBuffer, uint32_t InImageIndex);

	void RecreateSwapchain();
	void CleanupSwapchain();
	void WaitIdle();

	void Render();

	void Cleanup();

private:
	std::vector<const char*> ValidationLayers;
	std::vector<const char*> DeviceExtensions;

	VkInstance Instance;
	VkDebugUtilsMessengerEXT DebugMessenger;
	VkSurfaceKHR Surface;

	VkPhysicalDevice PhysicalDevice;
	VkDevice Device;

	VkQueue GraphicsQueue;
	VkQueue PresentQueue;

	VkSwapchainKHR Swapchain;
	std::vector<VkImage> SwapchainImages;
	VkFormat SwapchainImageFormat;
	VkExtent2D SwapchainExtent;
	std::vector<VkImageView> SwapchainImageViews;
	std::vector<VkFramebuffer> SwapchainFramebuffers;

	VkRenderPass RenderPass;
	VkPipelineLayout PipelineLayout;
	VkPipeline Pipeline;
	VkCommandPool CommandPool;

	VkBuffer VertexBuffer;
	VkDeviceMemory VertexBufferMemory;
	VkBuffer IndexBuffer;
	VkDeviceMemory IndexBufferMemory;

	std::vector<VkCommandBuffer> CommandBuffers;

	std::vector<VkSemaphore> ImageAvailableSemaphores;
	std::vector<VkSemaphore> RenderFinishedSemaphores;
	std::vector<VkFence> Fences;

	uint32_t CurrentFrame;

	struct FVertex
	{
		glm::vec3 Position;
		glm::vec3 Color;
	};
	std::vector<FVertex> Vertices;
	std::vector<uint32_t> Indices;
};
