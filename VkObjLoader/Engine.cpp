#include "Engine.h"
#include "Config.h"
#include "Renderer.h"
#include "Camera.h"

#include "glfw/glfw3.h"

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

	Renderer = new FSingleObjectRenderer(Window);
	Camera = new FCamera();
}

FEngine::~FEngine()
{
	delete Renderer;
	delete Camera;

	glfwDestroyWindow(Window);
	glfwTerminate();
}

GLFWwindow* FEngine::GetWindow() const
{
	return Window;
}

FRenderer* FEngine::GetRenderer() const
{
	return Renderer;
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

	glfwSetInputMode(Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}
