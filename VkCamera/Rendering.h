#pragma once

#include "vulkan/vulkan.h"
#include "glfw/glfw3.h"

#include "glm/glm.hpp"

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

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

#define MAX_CONCURRENT_FRAME 2

extern GLFWwindow* GWindow;

extern VkInstance GInstance;
extern VkDebugUtilsMessengerEXT GDebugMessenger;
extern VkSurfaceKHR GSurface;

extern VkPhysicalDevice GPhysicalDevice;
extern VkDevice GDevice;

extern VkQueue GGraphicsQueue;
extern VkQueue GPresentQueue;

extern VkSwapchainKHR GSwapchain;
extern std::vector<VkImage> GSwapchainImages;
extern VkFormat GSwapchainImageFormat;
extern VkExtent2D GSwapchainExtent;
extern std::vector<VkImageView> GSwapchainImageViews;
extern std::vector<VkFramebuffer> GSwapchainFramebuffers;

extern VkRenderPass GRenderPass;
extern VkPipelineLayout GPipelineLayout;
extern VkPipeline GPipeline;
extern VkCommandPool GCommandPool;

extern VkDescriptorPool GDescriptorPool;
extern VkDescriptorSetLayout GDescriptorSetLayout;
extern std::vector<VkDescriptorSet> GDescriptorSets;

extern VkImage GTextureImage;
extern VkDeviceMemory GTextureImageMemory;
extern VkImageView GTextureImageView;
extern VkSampler GTextureSampler;

extern VkBuffer GVertexBuffer;
extern VkDeviceMemory GVertexBufferMemory;

extern VkBuffer GIndexBuffer;
extern VkDeviceMemory GIndexBufferMemory;

struct FUniformBuffer
{
	VkBuffer Buffer;
	VkDeviceMemory Memory;
	void* Mapped;
};
extern std::vector<FUniformBuffer> GUniformBuffers;

extern std::vector<VkCommandBuffer> GCommandBuffers;

extern std::vector<VkSemaphore> GImageAvailableSemaphores;
extern std::vector<VkSemaphore> GRenderFinishedSemaphores;
extern std::vector<VkFence> GFences;

extern bool GFramebufferResized;

extern uint32_t GCurrentFrame;

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

extern const std::vector<FVertex> GVertices;
extern const std::vector<uint16_t> GIndices;

void InitializeGLFW();
void CreateGLFWWindow();
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
void WaitIdle();
void UpdateUniformBuffer();

void Render(float InDeltaTime);

void Cleanup();
