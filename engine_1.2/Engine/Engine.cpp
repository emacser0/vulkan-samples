#include "Engine.h"
#include "Config.h"
#include "AssetManager.h"
#include "Utils.h"
#include "World.h"
#include "LightActor.h"
#include "MeshActor.h" 

#include "VulkanContext.h"
#include "VulkanMeshRenderer.h"
#include "VulkanUIRenderer.h"

#include "glfw/glfw3.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_vulkan.h"
#include "imgui/imgui_impl_glfw.h"

#include <stdexcept>
#include <cassert>
#include <filesystem>

FEngine* GEngine;

void FEngine::Init()
{
	GEngine = new FEngine();
	GEngine->Initialize();
}

void FEngine::Exit()
{
	assert(GEngine != nullptr);
	delete GEngine;

	GEngine = nullptr;
}

FEngine::FEngine()
	: Window(nullptr)
	, World(nullptr)
	, RenderContext(nullptr)
	, MeshRenderer(nullptr)
	, UIRenderer(nullptr)
{
}

FEngine::~FEngine()
{
	delete World;
	FAssetManager::Shutdown();

	delete RenderContext;

	glfwDestroyWindow(Window);
	glfwTerminate();
}

void FEngine::Initialize()
{
	InitializeGLFW();
	CreateGLFWWindow();
	CompileShaders();

	World = new FWorld();
	RenderContext = new FVulkanContext(Window);
	MeshRenderer = RenderContext->CreateObject<FVulkanMeshRenderer>();
	UIRenderer = RenderContext->CreateObject<FVulkanUIRenderer>();

	FAssetManager::Startup();
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

FVulkanMeshRenderer* FEngine::GetMeshRenderer() const
{
	return MeshRenderer;
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
		MeshRenderer->SetScene(World->GetRenderScene());
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

	std::string ApplicationName;
	GConfig->Get("ApplicationName", ApplicationName);

	Window = glfwCreateWindow(WindowWidth, WindowHeight, ApplicationName.c_str(), nullptr, nullptr);
	if (Window == nullptr)
	{
		throw std::runtime_error("Failed to create window");
	}

	glfwSetMouseButtonCallback(Window, OnMouseButtonEvent);
	glfwSetScrollCallback(Window, OnMouseWheelEvent);
	glfwSetKeyCallback(Window, OnKeyEvent);
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
	FWorld* World = GEngine->GetWorld();
	if (World == nullptr)
	{
		return;
	}

	for (AActor* Actor : World->GetActors())
	{
		if (InAction == GLFW_PRESS)
		{
			Actor->OnMouseButtonDown(InButton, InMods);
		}
		else if (InAction == GLFW_RELEASE)
		{
			Actor->OnMouseButtonUp(InButton, InMods);
		}
	}
}

void FEngine::OnMouseWheelEvent(GLFWwindow* InWindow, double InXOffset, double InYOffset)
{
	assert(GEngine != nullptr);
	FWorld* World = GEngine->GetWorld();
	if (World == nullptr)
	{
		return;
	}

	for (AActor* Actor : World->GetActors())
	{
		Actor->OnMouseWheel(InXOffset, InYOffset);
	}
}

void FEngine::OnKeyEvent(GLFWwindow* InWindow, int InKey, int InScanCode, int InAction, int InMods)
{
	assert(GEngine != nullptr);
	FWorld* World = GEngine->GetWorld();
	if (World == nullptr)
	{
		return;
	}

	for (AActor* Actor : World->GetActors())
	{
		if (InAction == GLFW_PRESS)
		{
			Actor->OnKeyDown(InKey, InScanCode, InMods);
		}
		else if (InAction == GLFW_RELEASE)
		{
			Actor->OnKeyUp(InKey, InScanCode, InMods);
		}
	}
}
