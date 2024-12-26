#include "Engine.h"
#include "Config.h"
#include "Utils.h"
#include "Rendering.h"
#include "Camera.h"

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
	static bool bInitialized = false;
	static double PrevMouseX, PrevMouseY;

	glm::vec3 RotationDelta(0.0f);

	double MouseX, MouseY;
	glfwGetCursorPos(GWindow, &MouseX, &MouseY);

	FCamera* Camera = GEngine->GetCamera();
	assert(Camera != nullptr);

	if (bInitialized)
	{
		double MouseDeltaX = MouseX - PrevMouseX;
		double MouseDeltaY = MouseY - PrevMouseY;

		float MouseSensitivity;
		GConfig->Get("MouseSensitivity", MouseSensitivity);

		float PitchAmount = MouseDeltaY * MouseSensitivity * InDeltaTime;
		float YawAmount = -MouseDeltaX * MouseSensitivity * InDeltaTime;

		Camera->SetPitch(Camera->GetPitch() + PitchAmount);
		Camera->SetYaw(Camera->GetYaw() + YawAmount);
	}

	PrevMouseX = MouseX;
	PrevMouseY = MouseY;

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

	FTransform CameraTransform = Camera->GetTransform();

	glm::mat4 RotationMatrix = glm::toMat4(glm::quat(glm::vec3(Camera->GetPitch(), Camera->GetYaw(), Camera->GetRoll())));

	glm::vec4 MoveVector = RotationMatrix * glm::vec4(glm::normalize(MoveDelta), 1.0f);
	glm::vec3 FinalMoveDelta(MoveVector);
	FinalMoveDelta = glm::normalize(FinalMoveDelta);

	float CameraMoveSpeed;
	GConfig->Get("CameraMoveSpeed", CameraMoveSpeed);

	if (glm::length(FinalMoveDelta) > FLT_EPSILON)
	{
		CameraTransform.SetTranslation(CameraTransform.GetTranslation() + FinalMoveDelta * CameraMoveSpeed * InDeltaTime);
	}

	Camera->SetTransform(CameraTransform);

	bInitialized = true;
}

void Run(int argc, char** argv)
{
	FEngine::Init();

	std::string SolutionDirectory = SOLUTION_DIRECTORY;
	std::string ProjectDirectory = SolutionDirectory + PROJECT_NAME "/";

	GConfig->Set("ApplicationName", PROJECT_NAME);
	GConfig->Set("EngineName", "No Engine");
	GConfig->Set("WindowWidth", 800);
	GConfig->Set("WindowHeight", 600);
	GConfig->Set("TargetFPS", 60.0f);
	GConfig->Set("MaxConcurrentFrames", 2);
	GConfig->Set("MouseSensitivity", 1.0f);
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

	float TargetFPS;
	GConfig->Get("TargetFPS", TargetFPS);

	clock_t PreviousFrameTime = clock();
	float MaxFrameTime = 1000.0f / TargetFPS;

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

	FEngine::Exit();
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
