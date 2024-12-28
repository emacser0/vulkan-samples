#include "Engine.h"
#include "Config.h"
#include "Application.h"
#include "Camera.h"

#include "glfw/glfw3.h"

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

	Camera = std::make_shared<FCamera>();

	glfwSetMouseButtonCallback(Window, OnMouseButtonEvent);
	glfwSetScrollCallback(Window, OnMouseWheelEvent);
	glfwSetKeyCallback(Window, OnKeyEvent);
}

FEngine::~FEngine()
{
	if (Application != nullptr)
	{
		Application->Terminate();
	}

	glfwDestroyWindow(Window);
	glfwTerminate();
}

void FEngine::Run(std::shared_ptr<FApplication> InApplication)
{
	Application = InApplication;
	Application->Run();
}

void FEngine::Tick(float InDeltaTime)
{
	if (Application != nullptr)
	{
		Application->Tick(InDeltaTime);
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

