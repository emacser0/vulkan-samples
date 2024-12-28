#pragma once

#define GLFW_INCLUDE_VULKAN
#include "glfw/glfw3.h"

#include "glm/glm.hpp"

#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <stdexcept>
#include <algorithm>
#include <limits>
#include <cstddef>
#include <cassert>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

#define MAX_CONCURRENT_FRAME 2

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

extern VkBuffer GVertexBuffer;
extern VkDeviceMemory GVertexBufferMemory;
extern VkBuffer GIndexBuffer;
extern VkDeviceMemory GIndexBufferMemory;

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
};

extern const std::vector<FVertex> GVertices;
extern const std::vector<uint32_t> GIndices;

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

void RecreateSwapchain();
void CleanupSwapchain();
void WaitIdle();

void Render();

void Cleanup();
