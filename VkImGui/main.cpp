#include "Engine.h"
#include "Config.h"
#include "VkImGuiApplication.h"

#include <iostream>
#include <stdexcept>

void Run(int argc, char** argv)
{
	FConfig::Startup();

	std::string SolutionDirectory = SOLUTION_DIRECTORY;
	std::string ProjectDirectory = SolutionDirectory + PROJECT_NAME "/";

	GConfig->Set("ApplicationName", PROJECT_NAME);
	GConfig->Set("EngineName", "No Engine");
	GConfig->Set("WindowWidth", 800);
	GConfig->Set("WindowHeight", 600);
	GConfig->Set("TargetFPS", 60.0f);
	GConfig->Set("MaxConcurrentFrames", 2);
	GConfig->Set("MouseSensitivity", 0.5f);
	GConfig->Set("CameraMoveSpeed", 1.0f);
	GConfig->Set("ShaderDirectory", ProjectDirectory + "shaders/");
	GConfig->Set("ImageDirectory", SolutionDirectory + "resources/images/");
	GConfig->Set("MeshDirectory", SolutionDirectory + "resources/meshes/");

	FEngine::Init();

	std::shared_ptr<FApplication> Application = std::make_shared<FVkImGuiApplication>();
	GEngine->Run(Application);

	FEngine::Exit();
	FConfig::Shutdown();
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
