#include "Engine.h"
#include "Config.h"
#include "Utils.h"
#include "Camera.h"
#include "AssetManager.h"
#include "TextureSource.h"
#include "Mesh.h"

#include "VulkanContext.h"
#include "VulkanModel.h"
#include "VulkanScene.h"
#include "VulkanMeshRenderer.h"
#include "VulkanUIRenderer.h"
#include "VulkanTexture.h"

#include <ctime>
#include <chrono>
#include <thread>
#include <iostream>
#include <stdexcept>
#include <filesystem>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/quaternion.hpp"

glm::vec3 CameraMoveDelta(0.0f);

double PrevMouseX, PrevMouseY;

void OnMouseButtonEvent(GLFWwindow* Window, int Button, int Action, int Mods)
{
	if (Button == GLFW_MOUSE_BUTTON_RIGHT)
	{
		if (Action == GLFW_PRESS)
		{
			glfwSetInputMode(Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			glfwGetCursorPos(Window, &PrevMouseX, &PrevMouseY);
		}
		else if (Action == GLFW_RELEASE)
		{
			glfwSetInputMode(Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
	}
}

void OnMouseWheelEvent(GLFWwindow* Window, double XOffset, double YOffset)
{
	if (abs(YOffset) >= DBL_EPSILON)
	{
		FCamera* Camera = GEngine->GetCamera();
		assert(Camera != nullptr);

		FTransform CameraTransform = Camera->GetTransform();
		glm::mat4 RotationMatrix = glm::toMat4(CameraTransform.GetRotation());

		glm::vec4 MoveVector(0.0f, 0.0f, -YOffset, 1.0f);

		float CameraMoveSpeed;
		GConfig->Get("CameraMoveSpeed", CameraMoveSpeed);

		CameraTransform.SetTranslation(CameraTransform.GetTranslation() + glm::vec3(RotationMatrix * MoveVector) * CameraMoveSpeed * 0.1f);
		Camera->SetTransform(CameraTransform);
	}
}

void OnKeyEvent(GLFWwindow* Window, int Key, int ScanCode, int Action, int Mods)
{
	if (Key == GLFW_KEY_W)
	{
		if (Action == GLFW_PRESS)
		{
			CameraMoveDelta.z = -1.0f;
		}
		else if (Action == GLFW_RELEASE && CameraMoveDelta.z == -1.0f)
		{
			CameraMoveDelta.z = 0.0f;
		}
	}
	else if (Key == GLFW_KEY_S)
	{
		if (Action == GLFW_PRESS)
		{
			CameraMoveDelta.z = 1.0f;
		}
		else if (Action == GLFW_RELEASE && CameraMoveDelta.z == 1.0f)
		{
			CameraMoveDelta.z = 0.0f;
		}
	}
	else if (Key == GLFW_KEY_A)
	{
		if (Action == GLFW_PRESS)
		{
			CameraMoveDelta.x = -1.0f;
		}
		else if (Action == GLFW_RELEASE && CameraMoveDelta.x == -1.0f)
		{
			CameraMoveDelta.x = 0.0f;
		}
	}
	else if (Key == GLFW_KEY_D)
	{
		if (Action == GLFW_PRESS)
		{
			CameraMoveDelta.x = 1.0f;
		}
		else if (Action == GLFW_RELEASE && CameraMoveDelta.x == 1.0f)
		{
			CameraMoveDelta.x = 0.0f;
		}
	}
}

FVulkanMeshRenderer* MeshRenderer;

void Update(float InDeltaTime); 

int RandRange(int Min, int Max)
{
	return rand() % (Max - Min + 1) + Min;
}

void Run(int argc, char** argv)
{
	srand(static_cast<unsigned int>(time(NULL)));

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

	FEngine::Init();

	std::string ShaderDirectory;
	GConfig->Get("ShaderDirectory", ShaderDirectory);

	GLFWwindow* Window = GEngine->GetWindow();

	glfwSetMouseButtonCallback(Window, OnMouseButtonEvent);
	glfwSetScrollCallback(Window, OnMouseWheelEvent);
	glfwSetKeyCallback(Window, OnKeyEvent);

	FVulkanUIRenderer* UIRenderer = GEngine->GetUIRenderer();
	UIRenderer->Ready();

	std::string MeshDirectory;
	GConfig->Get("MeshDirectory", MeshDirectory);

	std::string ImageDirectory;
	GConfig->Get("ImageDirectory", ImageDirectory);

	FVulkanContext* RenderContext = GEngine->GetRenderContext();

	FTextureSource* TextureSource = FAssetManager::CreateAsset<FTextureSource>();
	TextureSource->Load(ImageDirectory + "wood.jpg");

	FVulkanTexture* Texture = new FVulkanTexture(RenderContext);
	Texture->LoadSource(*TextureSource);

	FMesh* CubeMeshAsset = FAssetManager::CreateAsset<FMesh>();
	CubeMeshAsset->Load(MeshDirectory + "cube.obj");

	FVulkanMesh* Mesh = RenderContext->CreateObject<FVulkanMesh>();
	Mesh->Load(CubeMeshAsset);
	Mesh->SetTexture(Texture);

	for (int32_t Idx = 0; Idx < 100; ++Idx)
	{
		FVulkanModel* Model = new FVulkanModel(RenderContext);
		Model->SetMesh(Mesh);

		FTransform ModelTransform = Model->GetTransform();
		ModelTransform.SetTranslation(glm::vec3(RandRange(-50, 50), RandRange(-50, 50), RandRange(-50, 50)));
		ModelTransform.SetRotation(glm::vec3(glm::radians(90.0f), 0.0f, 0.0f));
		ModelTransform.SetScale(glm::vec3(1.0f, 1.0f, 1.0f));

		Model->SetTransform(ModelTransform);

		GEngine->GetScene()->AddModel(Model);
	}

	MeshRenderer = RenderContext->CreateObject<FVulkanMeshRenderer>();
	MeshRenderer->Ready();

	float TargetFPS;
	GConfig->Get("TargetFPS", TargetFPS);

	clock_t PreviousFrameTime = clock();
	float MaxFrameTime = 1000.0f / TargetFPS;

	while (!glfwWindowShouldClose(Window))
	{
		clock_t CurrentFrameTime = clock();
		float DeltaTime = static_cast<float>(CurrentFrameTime - PreviousFrameTime) / CLOCKS_PER_SEC;

		glfwPollEvents();
		Update(DeltaTime);

		RenderContext->BeginRender();
		MeshRenderer->Render();
		UIRenderer->Render();
		RenderContext->EndRender();

		PreviousFrameTime = CurrentFrameTime;

		std::this_thread::sleep_for(std::chrono::milliseconds((int)(MaxFrameTime)));
	}

	MeshRenderer->WaitIdle();

	delete Mesh;
	delete Texture;

	FEngine::Exit();
	FConfig::Shutdown();
}

void Update(float InDeltaTime)
{
	GLFWwindow* Window = GEngine->GetWindow();
	if (glfwGetMouseButton(Window, GLFW_MOUSE_BUTTON_RIGHT) != GLFW_PRESS)
	{
		return;
	}

	glm::vec3 RotationDelta(0.0f);

	double MouseX, MouseY;
	glfwGetCursorPos(Window, &MouseX, &MouseY);

	FCamera* Camera = GEngine->GetCamera();
	assert(Camera != nullptr);

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

	PrevMouseX = MouseX;
	PrevMouseY = MouseY;

	FTransform CameraTransform = Camera->GetTransform();

	glm::mat4 RotationMatrix = glm::toMat4(CameraTransform.GetRotation());

	glm::vec4 MoveVector = RotationMatrix * glm::vec4(glm::normalize(CameraMoveDelta), 1.0f);
	glm::vec3 FinalMoveDelta(MoveVector);
	FinalMoveDelta = glm::normalize(FinalMoveDelta);

	float CameraMoveSpeed;
	GConfig->Get("CameraMoveSpeed", CameraMoveSpeed);

	if (glm::length(FinalMoveDelta) > FLT_EPSILON)
	{
		CameraTransform.SetTranslation(CameraTransform.GetTranslation() + FinalMoveDelta * CameraMoveSpeed * InDeltaTime);
	}

	Camera->SetTransform(CameraTransform);
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

