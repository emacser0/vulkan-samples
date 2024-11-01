#include "Rendering.h"
#include "Config.h"

void Run(int argc, char** argv)
{	
	FConfig::Startup();

	std::string ProjectName = "VkFirstProgram";
	std::string ProjectDirectory = SOLUTION_DIRECTORY + ProjectName + "/";

	GConfig->Set("ApplicationName", ProjectName);
	GConfig->Set("EngineName", "No Engine");
	GConfig->Set("WindowWidth", 800);
	GConfig->Set("WindowHeight", 600);
	GConfig->Set("WindowTitle", ProjectName);
	GConfig->Set("TargetFPS", 60.0f);
	GConfig->Set("MaxConcurrentFrames", 2);
	GConfig->Set("MouseSensitivity", 0.5f);
	GConfig->Set("CameraMoveSpeed", 1.0f);
	GConfig->Set("ShaderDirectory", ProjectDirectory + "Shaders/");
	GConfig->Set("ImageDirectory", ProjectDirectory + "Images/");
	GConfig->Set("ResourceDirectory", ProjectDirectory + "Resources/");

	InitializeGLFW();
	CreateWindow();
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

	while (!glfwWindowShouldClose(GWindow))
	{
		glfwPollEvents();
		Render();
	}

	WaitIdle();
	Cleanup();
}

int main(int argc, char** argv)
{
	try
	{
		Run(argc, argv);
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}