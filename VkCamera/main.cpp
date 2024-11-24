#include "Rendering.h"
#include "Camera.h"
#include "Config.h"

#include <ctime>
#include <chrono>
#include <thread>
#include <filesystem>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/quaternion.hpp"

void Update(float InDeltaTime)
{
	static double PrevMouseX = 0.0f;
	static double PrevMouseY = 0.0f;

	double MouseX, MouseY;
	glfwGetCursorPos(GWindow, &MouseX, &MouseY);

	glm::vec3 RotationDelta(0.0f);

	if (glfwGetKey(GWindow, GLFW_KEY_UP) == GLFW_PRESS)
	{
		RotationDelta.x = 1.0f;
	}

	if (glfwGetKey(GWindow, GLFW_KEY_DOWN) == GLFW_PRESS)
	{
		RotationDelta.x = -1.0f;
	}

	if (glfwGetKey(GWindow, GLFW_KEY_LEFT) == GLFW_PRESS)
	{
		RotationDelta.y = -1.0f;
	}

	if (glfwGetKey(GWindow, GLFW_KEY_RIGHT) == GLFW_PRESS)
	{
		RotationDelta.y = 1.0f;
	}

	if (glm::length(RotationDelta) > FLT_EPSILON)
	{
		GCamera.Rotation += glm::normalize(RotationDelta) * GCamera.MouseSensitivity * InDeltaTime;
	}

	glm::vec3 MoveDelta(0.0f);

	if (glfwGetKey(GWindow, GLFW_KEY_W) == GLFW_PRESS)
	{
		MoveDelta.z = -1.0f;
	}

	if (glfwGetKey(GWindow, GLFW_KEY_S) == GLFW_PRESS)
	{
		MoveDelta.z = 1.0f;
	}

	if (glfwGetKey(GWindow, GLFW_KEY_A) == GLFW_PRESS)
	{
		MoveDelta.x = -1.0f;
	}

	if (glfwGetKey(GWindow, GLFW_KEY_D) == GLFW_PRESS)
	{
		MoveDelta.x = 1.0f;
	}

	glm::quat Rotation(GCamera.Rotation);
	glm::mat4 RotationMatrix = glm::transpose(glm::toMat4(Rotation));

	glm::vec4 MoveVector = RotationMatrix * glm::vec4(glm::normalize(MoveDelta), 1.0f);
	glm::vec3 FinalMoveDelta(MoveVector);

	if (glm::length(FinalMoveDelta) > FLT_EPSILON)
	{
		GCamera.Position += FinalMoveDelta * GCamera.MoveSpeed * InDeltaTime;
	}
}

void CompileShaders(const std::string& InDirectory)
{
	for (const auto& Entry : std::filesystem::directory_iterator(InDirectory))
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

void Run(int argc, char** argv)
{
	FConfig::Startup();

	std::string SolutionDirectory = SOLUTION_DIRECTORY;
	std::string ProjectDirectory = SolutionDirectory + PROJECT_NAME "/";

	GConfig->Set("ApplicationName", PROJECT_NAME);
	GConfig->Set("EngineName", "No Engine");
	GConfig->Set("WindowWidth", 800);
	GConfig->Set("WindowHeight", 600);
	GConfig->Set("WindowTitle", PROJECT_NAME);
	GConfig->Set("TargetFPS", 60.0f);
	GConfig->Set("MaxConcurrentFrames", 2);
	GConfig->Set("MouseSensitivity", 0.5f);
	GConfig->Set("CameraMoveSpeed", 1.0f);
	GConfig->Set("ShaderDirectory", ProjectDirectory + "shaders/");
	GConfig->Set("ImageDirectory", SolutionDirectory + "resources/images/");
	GConfig->Set("MeshDirectory", SolutionDirectory + "resources/meshes/");

	std::string ShaderDirectory;
	GConfig->Get("ShaderDirectory", ShaderDirectory);

	CompileShaders(ShaderDirectory);

	InitializeGLFW();
	CreateGLFWWindow();
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

	InitializeCamera();

	clock_t PreviousFrameTime = clock();
	float MaxFrameTime = 1000.0f / 60.0f;

	while (!glfwWindowShouldClose(GWindow))
	{
		clock_t CurrentFrameTime = clock();
		float DeltaTime = static_cast<float>(CurrentFrameTime - PreviousFrameTime) / CLOCKS_PER_SEC;

		glfwPollEvents();
		Update(DeltaTime);
		Render(DeltaTime);

		PreviousFrameTime = CurrentFrameTime;

		std::this_thread::sleep_for(std::chrono::milliseconds((int)(MaxFrameTime)));
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
