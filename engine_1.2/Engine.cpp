#include "Engine.h"
#include "Config.h"
#include "AssetManager.h"
#include "Utils.h"
#include "World.h"
#include "LightActor.h"
#include "MeshActor.h" 

#include "VulkanContext.h"
#include "VulkanUIRenderer.h"

#include "glfw/glfw3.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_vulkan.h"
#include "imgui/imgui_impl_glfw.h"

#include <stdexcept>

FEngine* GEngine;

void FEngine::Init()
{
	GEngine = new FEngine();
}

void FEngine::Exit()
{
	assert(GEngine != nullptr);
	delete GEngine;

	GEngine = nullptr;
}

FEngine::FEngine()
{
	InitializeGLFW();
	CreateGLFWWindow();

	World = new FWorld();
	RenderContext = new FVulkanContext(Window);
	UIRenderer = RenderContext->CreateObject<FVulkanUIRenderer>();

	FAssetManager::Startup();
}

FEngine::~FEngine()
{
	delete RenderContext;
	delete World;

	glfwDestroyWindow(Window);
	glfwTerminate();

	FAssetManager::Shutdown();
}

GLFWwindow* FEngine::GetWindow() const
{
	return Window;
}

FWorld* FEngine::GetWorld() const
{
	return World;
}

FVulkanContext* FEngine::GetRenderContext() const
{
	return RenderContext;
}

FVulkanUIRenderer* FEngine::GetUIRenderer() const
{
	return UIRenderer;
}

void FEngine::Tick(float DeltaTime)
{
	if (World != nullptr)
	{
		World->Tick(DeltaTime);
	}
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
