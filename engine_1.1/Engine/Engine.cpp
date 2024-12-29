#include "Engine.h"
#include "Config.h"
#include "Application.h"
#include "AssetManager.h"
#include "Widget.h"

#include "VulkanContext.h"
#include "VulkanScene.h"
#include "VulkanUIRenderer.h"

#include "glfw/glfw3.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_vulkan.h"
#include "imgui/imgui_impl_glfw.h"

#include <chrono>
#include <thread>
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

	glfwSetMouseButtonCallback(Window, OnMouseButtonEvent);
	glfwSetScrollCallback(Window, OnMouseWheelEvent);
	glfwSetKeyCallback(Window, OnKeyEvent);

	RenderContext = new FVulkanContext(Window);
	Scene = RenderContext->CreateObject<FVulkanScene>();
	UIRenderer = RenderContext->CreateObject<FVulkanUIRenderer>();

	FAssetManager::Startup();
}

FEngine::~FEngine()
{
	if (Application != nullptr)
	{
		Application->Terminate();
		Application = nullptr;
	}

	RenderContext = nullptr;
	Scene = nullptr;
	UIRenderer = nullptr;

	glfwDestroyWindow(Window);
	glfwTerminate();

	FAssetManager::Shutdown();
}

void FEngine::Run(std::shared_ptr<FApplication> InApplication)
{
	Application = InApplication;
	Application->Run();

	float TargetFPS;
	GConfig->Get("TargetFPS", TargetFPS);

	clock_t PreviousFrameTime = clock();
	float MaxFrameTime = 1000.0f / TargetFPS;

	GLFWwindow* Window = GEngine->GetWindow();
	assert(Window != nullptr);

	while (!glfwWindowShouldClose(Window))
	{
		clock_t CurrentFrameTime = clock();
		float DeltaTime = static_cast<float>(CurrentFrameTime - PreviousFrameTime) / CLOCKS_PER_SEC;

		glfwPollEvents();

		Tick(DeltaTime);

		PreviousFrameTime = CurrentFrameTime;

		std::this_thread::sleep_for(std::chrono::milliseconds((int)(MaxFrameTime)));
	}
}

void FEngine::Tick(float InDeltaTime)
{
	if (Application != nullptr)
	{
		Application->Tick(InDeltaTime);
	}
}

void FEngine::AddWidget(const std::shared_ptr<FWidget>& InWidget)
{
	Widgets.push_back(InWidget);
}

void FEngine::RemoveWidget(const std::shared_ptr<FWidget>& InWidget)
{
	std::remove_if(Widgets.begin(), Widgets.end(), [InWidget](std::shared_ptr<FWidget> Widget)
	{
	   return Widget == InWidget;
	});
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

	std::string ApplicationName;
	GConfig->Get("ApplicationName", ApplicationName);

	Window = glfwCreateWindow(WindowWidth, WindowHeight, ApplicationName.c_str(), nullptr, nullptr);
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

void FEngine::OnMouseButtonEvent(GLFWwindow* InWindow, int InButton, int InAction, int InMods)
{
	assert(GEngine != nullptr);

	std::shared_ptr<FApplication> Application = GEngine->GetApplication();
	assert(Application != nullptr);

	if (InAction == GLFW_PRESS)
	{
		Application->OnMouseButtonDown(InButton, InMods);
	}
	else if (InAction == GLFW_RELEASE)
	{
		Application->OnMouseButtonUp(InButton, InMods);
	}
}

void FEngine::OnMouseWheelEvent(GLFWwindow* InWindow, double InXOffset, double InYOffset)
{
	assert(GEngine != nullptr);

	std::shared_ptr<FApplication> Application = GEngine->GetApplication();
	assert(Application != nullptr);

	Application->OnMouseWheel(InXOffset, InYOffset);
}

void FEngine::OnKeyEvent(GLFWwindow* InWindow, int InKey, int InScanCode, int InAction, int InMods)
{
	assert(GEngine != nullptr);

	std::shared_ptr<FApplication> Application = GEngine->GetApplication();
	assert(Application != nullptr);
	if (InAction == GLFW_PRESS)
	{
		Application->OnKeyDown(InKey, InScanCode, InMods);
	}
	else if (InAction == GLFW_RELEASE)
	{
		Application->OnKeyUp(InKey, InScanCode, InMods);
	}
}

