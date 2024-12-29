#pragma once

#include "vulkan/vulkan.h"
#include "glfw/glfw3.h"

#include "glm/glm.hpp"

#include "Math.h"
#include "Texture.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <set>
#include <stdexcept>
#include <algorithm>
#include <limits>
#include <cstddef>
#include <cassert>

#define MAX_CONCURRENT_FRAME 2

struct FUniformBuffer
{
	VkBuffer Buffer;
	VkDeviceMemory Memory;
	void* Mapped;
};

struct FUniformBufferObject
{
	alignas(16) glm::mat4 Model;
	alignas(16) glm::mat4 View;
	alignas(16) glm::mat4 Projection;
};

class FSingleObjectRenderer
{
public:
	FSingleObjectRenderer();
	virtual ~FSingleObjectRenderer();

	std::vector<FVertex>& GetVertices() { return Vertices; }
	std::vector<uint32_t>& GetIndices() { return Indices;  }
	FTexture& GetTexture() { return Texture; }

	void Render(float InDeltaTime);
	void Ready();

	void WaitIdle();

	void SetViewMatrix(const glm::mat4& InViewMatrix) { ViewMatrix = InViewMatrix; }
	void SetProjectionMatrix(const glm::mat4& InProjectionMatrix) { ProjectionMatrix = InProjectionMatrix; }

protected:
	void RecordCommandBuffer(VkCommandBuffer InCommandBuffer, uint32_t InImageIndex);

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

	void RecreateSwapchain();
	void CleanupSwapchain();

	void UpdateUniformBuffer();

protected:
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

	FTexture Texture;
	VkImage TextureImage;
	VkDeviceMemory TextureImageMemory;
	VkImageView TextureImageView;
	VkSampler TextureSampler;

	VkBuffer VertexBuffer;
	VkDeviceMemory VertexBufferMemory;

	VkBuffer IndexBuffer;
	VkDeviceMemory IndexBufferMemory;

	std::vector<FUniformBuffer> UniformBuffers;
	glm::mat4 ViewMatrix;
	glm::mat4 ProjectionMatrix;

	std::vector<VkCommandBuffer> CommandBuffers;

	std::vector<VkSemaphore> ImageAvailableSemaphores;
	std::vector<VkSemaphore> RenderFinishedSemaphores;
	std::vector<VkFence> Fences;

	uint32_t CurrentFrame;

	std::vector<FVertex> Vertices;
	std::vector<uint32_t> Indices;

};

