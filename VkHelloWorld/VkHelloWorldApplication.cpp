#include "VkHelloWorldApplication.h"
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

void FVkHelloWorldApplication::Run()
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
	CreateGraphicsPipeline();
	CreateFramebuffers();
	CreateCommandPool();
	CreateVertexBuffer();
	CreateIndexBuffer();
	CreateCommandBuffers();
	CreateSyncObjects();
}

void FVkHelloWorldApplication::Terminate()
{
	WaitIdle();
	Cleanup();
}

void FVkHelloWorldApplication::Tick(float InDeltaTime)
{
	Render();
}
