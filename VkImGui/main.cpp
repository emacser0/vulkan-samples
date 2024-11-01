#include "Engine.h"
#include "Config.h"
#include "VulkanContext.h"
#include "VulkanMeshRenderer.h"
#include "VulkanUIRenderer.h"
#include "Camera.h"
#include "Utils.h"
#include "MeshUtils.h"

#include <ctime>
#include <chrono>
#include <thread>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/quaternion.hpp"

void Update(float InDeltaTime)
{
	static bool bInitialized = false;
	static double PrevMouseX, PrevMouseY;

	GLFWwindow* Window = GEngine->GetWindow();

	glm::vec3 RotationDelta(0.0f);

	double MouseX, MouseY;
	glfwGetCursorPos(Window, &MouseX, &MouseY);

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

		if (abs(PitchAmount) > FLT_EPSILON || abs(YawAmount) > FLT_EPSILON)
		{
			FTransform CameraTransform = Camera->GetTransform();
			glm::quat CameraRotation = CameraTransform.GetRotation();

			glm::quat PitchRotation = glm::angleAxis(PitchAmount, glm::vec3(1.0f, 0.0f, 0.0f));
			glm::quat YawRotation = glm::angleAxis(YawAmount, glm::vec3(0.0f, 1.0f, 0.0f));

			CameraTransform.SetRotation(YawRotation * CameraTransform.GetRotation() * PitchRotation);

			Camera->SetTransform(CameraTransform);
		}
	}

	PrevMouseX = MouseX;
	PrevMouseY = MouseY;

	glm::vec3 MoveDelta(0.0f);

	if (glfwGetKey(Window, GLFW_KEY_W) == GLFW_PRESS)
	{
		MoveDelta.z = -1.0f;
	}

	if (glfwGetKey(Window, GLFW_KEY_S) == GLFW_PRESS)
	{
		MoveDelta.z = 1.0f;
	}

	if (glfwGetKey(Window, GLFW_KEY_A) == GLFW_PRESS)
	{
		MoveDelta.x = -1.0f;
	}

	if (glfwGetKey(Window, GLFW_KEY_D) == GLFW_PRESS)
	{
		MoveDelta.x = 1.0f;
	}

	FTransform CameraTransform = Camera->GetTransform();

	glm::mat4 RotationMatrix = glm::toMat4(CameraTransform.GetRotation());

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
	FConfig::Startup();

	GConfig->Set("ApplicationName", "VkImgui");
	GConfig->Set("EngineName", "No Engine");
	GConfig->Set("WindowWidth", 800);
	GConfig->Set("WindowHeight", 600);
	GConfig->Set("WindowTitle", "VkImgui");
	GConfig->Set("TargetFPS", 60.0f);
	GConfig->Set("MaxConcurrentFrames", 2);
	GConfig->Set("MouseSensitivity", 0.5f);
	GConfig->Set("CameraMoveSpeed", 1.0f);
	GConfig->Set("ShaderDirectory", SOLUTION_DIRECTORY "VkImGui/Shaders/");
	GConfig->Set("ImageDirectory", SOLUTION_DIRECTORY "VkImGui/Images/");
	GConfig->Set("ResourceDirectory", SOLUTION_DIRECTORY "VkImGui/Resources/");

	FEngine::Init();

	FVulkanUIRenderer* UIRenderer = GEngine->GetUIRenderer();
	UIRenderer->Ready();

	std::string ResourceDirectory;
	GConfig->Get("ResourceDirectory", ResourceDirectory);

	FVulkanMeshRenderer* MeshRenderer = GEngine->GetMeshRenderer();

	LoadModel(ResourceDirectory + "viking_room.obj", MeshRenderer->GetVertices(), MeshRenderer->GetIndices());

	std::string ImageDirectory;
	GConfig->Get("ImageDirectory", ImageDirectory);

	MeshRenderer->GetTexture().Source.Load(ImageDirectory + "viking_room.png");
	MeshRenderer->Ready();

	float TargetFPS;
	GConfig->Get("TargetFPS", TargetFPS);

	clock_t PreviousFrameTime = clock();
	float MaxFrameTime = 1000.0f / TargetFPS;

	GLFWwindow* Window = GEngine->GetWindow();

	while (!glfwWindowShouldClose(Window))
	{
		clock_t CurrentFrameTime = clock();
		float DeltaTime = static_cast<float>(CurrentFrameTime - PreviousFrameTime) / CLOCKS_PER_SEC;

		glfwPollEvents();
		Update(DeltaTime);

		FVulkanContext* RenderContext = GEngine->GetRenderContext();
		RenderContext->BeginRender();
		MeshRenderer->Render();
		UIRenderer->Render();
		RenderContext->EndRender();

		PreviousFrameTime = CurrentFrameTime;

		std::this_thread::sleep_for(std::chrono::milliseconds((int)(MaxFrameTime)));
	}

	MeshRenderer->WaitIdle();

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
