#include "VkTextureApplication.h"
#include "Engine.h"
#include "Config.h"
#include "Utils.h"
#include "Rendering.h"

#include <ctime>
#include <chrono>
#include <thread>
#include <filesystem>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/quaternion.hpp"

void FVkTextureApplication::Run()
{
	AddResizeCallback();
	CreateInstance();
	SetupDebugMessenger();
	CreateSurface();
	PickPhysicalDevice();
	CreateLogicalDevice();
	CreateSwapchain();
	CreateImageViews();
	CreateRenderPass();
	CreateDescriptorSetLayout();
	CreateGraphicsPipeline();
	CreateFramebuffers();
	CreateCommandPool();
	CreateTextureImage();
	CreateTextureImageView();
	CreateTextureSampler();
	CreateVertexBuffer();
	CreateIndexBuffer();
	CreateUniformBuffers();
	CreateDescriptorPool();
	CreateDescriptorSets();
	CreateCommandBuffers();
	CreateSyncObjects();
}

void FVkTextureApplication::Terminate()
{
	WaitIdle();
	Cleanup();
}

void FVkTextureApplication::Tick(float InDeltaTime)
{
	Render();
}
