#include "VkCubeApplication.h"
#include "Engine.h"
#include "Config.h"
#include "Utils.h"
#include "Rendering.h"
#include "Camera.h"
#include "CameraController.h"

#include <ctime>
#include <chrono>
#include <thread>
#include <filesystem>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/quaternion.hpp"

void FVkCubeApplication::Run()
{
	GConfig->Set("CameraMoveSpeed", 1.0f);
	GConfig->Set("MouseSensitivity", 1.0f);

	CameraController = std::make_shared<FCameraController>();

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

void FVkCubeApplication::Terminate()
{
	WaitIdle();
	Cleanup();
}

void FVkCubeApplication::Tick(float InDeltaTime)
{
	CameraController->Tick(InDeltaTime);

	Render(InDeltaTime);
}

void FVkCubeApplication::OnMouseButtonDown(int InButton, int InMods)
{
	CameraController->OnMouseButtonDown(InButton, InMods);
}

void FVkCubeApplication::OnMouseButtonUp(int InButton, int InMods)
{
	CameraController->OnMouseButtonUp(InButton, InMods);
}

void FVkCubeApplication::OnMouseWheel(double InXOffset, double InYOffset)
{
	CameraController->OnMouseWheel(InXOffset, InYOffset);
}

void FVkCubeApplication::OnKeyDown(int InKey, int InScanCode, int InMods)
{
	CameraController->OnKeyDown(InKey, InScanCode, InMods);
}

void FVkCubeApplication::OnKeyUp(int InKey, int InScanCode, int InMods)
{
	CameraController->OnKeyUp(InKey, InScanCode, InMods);
}

