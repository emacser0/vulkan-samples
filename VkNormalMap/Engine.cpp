#include "Engine.h"
#include "Config.h"
#include "AssetManager.h"
#include "VulkanContext.h"
#include "VulkanScene.h"
#include "VulkanUIRenderer.h"
#include "VulkanMeshRenderer.h"
#include "Camera.h"

#include "glfw/glfw3.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_vulkan.h"
#include "imgui/imgui_impl_glfw.h"

#include <stdexcept>

FEngine* GEngine;

void FEngine::Init()
{
	GEngine = new FEngine();

	FAssetManager::Startup();
}

void FEngine::Exit()
{
	assert(GEngine != nullptr);
	delete GEngine;

	GEngine = nullptr;

	FAssetManager::Shutdown();
}

FEngine::FEngine()
{
	InitializeGLFW();
	CreateGLFWWindow();

	RenderContext = new FVulkanContext(Window);
	Scene = RenderContext->CreateObject<FVulkanScene>();
	UIRenderer = RenderContext->CreateObject<FVulkanUIRenderer>();
	MeshRenderer = RenderContext->CreateObject<FVulkanMeshRenderer>();
	Camera = new FCamera();
}

FEngine::~FEngine()
{
	delete RenderContext;
	delete Camera;

	glfwDestroyWindow(Window);
	glfwTerminate();
}

GLFWwindow* FEngine::GetWindow() const
{
	return Window;
}

FVulkanContext* FEngine::GetRenderContext() const
{
	return RenderContext;
}

FVulkanScene* FEngine::GetScene() const
{
	return Scene;
}

FVulkanUIRenderer* FEngine::GetUIRenderer() const
{
	return UIRenderer;
}

FVulkanMeshRenderer* FEngine::GetMeshRenderer() const
{
	return MeshRenderer;
}

FCamera* FEngine::GetCamera() const
{
	return Camera;
}

void FEngine::InitializeGLFW()
{
	if (glfwInit() == 0)
	{
		throw std::runtime_error("Failed to initialize glfw");
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
}

void FEngine::CreateGLFWWindow()
{
	int32_t WindowWidth;
	int32_t WindowHeight;
	GConfig->Get("WindowWidth", WindowWidth);
	GConfig->Get("WindowHeight", WindowHeight);

	std::string WindowTitle;
	GConfig->Get("WindowTitle", WindowTitle);

	Window = glfwCreateWindow(WindowWidth, WindowHeight, WindowTitle.c_str(), nullptr, nullptr);
	if (Window == nullptr)
	{
		throw std::runtime_error("Failed to create window");
	}
}
