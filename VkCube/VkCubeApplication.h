#pragma once

#include "Application.h"

#include "vulkan/vulkan.h"
#include "glm/glm.hpp"

#include <memory>
#include <vector>

class FVkCubeApplication : public FApplication
{
public:
	FVkCubeApplication();

	virtual void Run() override;
	virtual void Terminate() override;

	virtual void Tick(float InDeltaTime) override;

	virtual void OnMouseButtonDown(int InButton, int InMods) override;
	virtual void OnMouseButtonUp(int InButton, int InMods) override;
	virtual void OnMouseWheel(double InXOffset, double InYOffset) override;
	virtual void OnKeyDown(int InKey, int InScanCode, int InMods) override;
	virtual void OnKeyUp(int InKey, int InScanCode, int InMods) override;

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
	void UpdateUniformBuffer();

	void Render(float InDeltaTime);

	void Cleanup();

private:
	std::shared_ptr<class FCamera> Camera;
	std::shared_ptr<class FCameraController> CameraController;

	std::vector<const char*> GValidationLayers;
	std::vector<const char*> GDeviceExtensions;

	VkInstance GInstance;
	VkDebugUtilsMessengerEXT GDebugMessenger;
	VkSurfaceKHR GSurface;

	VkPhysicalDevice GPhysicalDevice;
	VkDevice GDevice;

	VkQueue GGraphicsQueue;
	VkQueue GPresentQueue;

	VkSwapchainKHR GSwapchain;
	std::vector<VkImage> GSwapchainImages;
	VkFormat GSwapchainImageFormat;
	VkExtent2D GSwapchainExtent;
	std::vector<VkImageView> GSwapchainImageViews;
	std::vector<VkFramebuffer> GSwapchainFramebuffers;

	VkRenderPass GRenderPass;
	VkPipelineLayout GPipelineLayout;
	VkPipeline GPipeline;
	VkCommandPool GCommandPool;

	VkDescriptorPool GDescriptorPool;
	VkDescriptorSetLayout GDescriptorSetLayout;
	std::vector<VkDescriptorSet> GDescriptorSets;

	VkImage GTextureImage;
	VkDeviceMemory GTextureImageMemory;
	VkImageView GTextureImageView;
	VkSampler GTextureSampler;

	VkBuffer GVertexBuffer;
	VkDeviceMemory GVertexBufferMemory;

	VkBuffer GIndexBuffer;
	VkDeviceMemory GIndexBufferMemory;

	struct FUniformBuffer
	{
		VkBuffer Buffer;
		VkDeviceMemory Memory;
		void* Mapped;
	};
	std::vector<FUniformBuffer> GUniformBuffers;

	std::vector<VkCommandBuffer> GCommandBuffers;

	std::vector<VkSemaphore> GImageAvailableSemaphores;
	std::vector<VkSemaphore> GRenderFinishedSemaphores;
	std::vector<VkFence> GFences;

	uint32_t GCurrentFrame;

	struct FVertex
	{
		glm::vec3 Position;
		glm::vec3 Color;
		glm::vec2 TexCoords;
	};

	struct FUniformBufferObject
	{
		alignas(16) glm::mat4 Model;
		alignas(16) glm::mat4 View;
		alignas(16) glm::mat4 Projection;
	};

	const std::vector<FVertex> GVertices;
	const std::vector<uint32_t> GIndices;
};
