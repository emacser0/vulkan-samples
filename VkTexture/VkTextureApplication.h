#pragma once

#include "Application.h"

#include "vulkan/vulkan.h"
#include "glm/glm.hpp"

#include <memory>
#include <vector>

class FVkTextureApplication : public FApplication
{
public:
	FVkTextureApplication();

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
	void CreateTextureImage();
	void CreateTextureImageView();
	void CreateTextureSampler();
	void CreateVertexBuffer();
	void CreateIndexBuffer();
	void CreateUniformBuffers();
	void CreateDescriptorPool();
	void CreateDescriptorSetLayout();
	void CreateDescriptorSets();
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

	VkDescriptorPool DescriptorPool;
	VkDescriptorSetLayout DescriptorSetLayout;
	std::vector<VkDescriptorSet> DescriptorSets;

	VkImage TextureImage;
	VkDeviceMemory TextureImageMemory;
	VkImageView TextureImageView;
	VkSampler TextureSampler;

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
		glm::vec2 TexCoords;
	};

	const std::vector<FVertex> Vertices;
	const std::vector<uint32_t> Indices;
};
