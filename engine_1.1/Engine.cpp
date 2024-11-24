#include "Engine.h"
#include "Config.h"
#include "AssetManager.h"
#include "VulkanContext.h"
#include "VulkanScene.h"
#include "VulkanUIRenderer.h"
#include "Camera.h"

#include "glfw/glfw3.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_vulkan.h"
#include "imgui/imgui_impl_glfw.h"

#include <stdexcept>
#include <filesystem>

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
	CompileShaders();

	RenderContext = new FVulkanContext(Window);
	Scene = RenderContext->CreateObject<FVulkanScene>();
	UIRenderer = RenderContext->CreateObject<FVulkanUIRenderer>();
	Camera = new FCamera();

	FAssetManager::Startup();
}

FEngine::~FEngine()
{
	delete RenderContext;
	delete Camera;

	glfwDestroyWindow(Window);
	glfwTerminate();

	FAssetManager::Shutdown();
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

void FEngine::CompileShaders()
{
	std::string ShaderDirectory;
	GConfig->Get("ShaderDirectory", ShaderDirectory);

	for (const auto& Entry : std::filesystem::directory_iterator(ShaderDirectory))
	{
		std::string Filename = Entry.path().string();
		std::string Extension = Entry.path().extension().string();
		if (Extension == ".vert" || Extension == ".frag" || Extension == ".geom")
		{
			std::string Command = "glslang -g -V ";
			Command += Filename;
			Command += " -o ";
			Command += Filename + ".spv";

			system(Command.c_str());
		}
	}
}

