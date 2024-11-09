#include "Config.h"
#include "Mesh.h"
#include "AssetManager.h"
#include "TextureSource.h"
#include "Widget.h"

#include "VulkanContext.h"
#include "VulkanModel.h"
#include "VulkanScene.h"
#include "VulkanMeshRenderer.h"
#include "VulkanUIRenderer.h"

#include "Engine.h"
#include "World.h"
#include "Actor.h"
#include "CameraActor.h"
#include "LightActor.h"
#include "MeshActor.h"

#include <ctime>
#include <chrono>
#include <thread>
#include <iostream>
#include <stdexcept>
#include <filesystem>
#include <cstdlib>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/quaternion.hpp"

#include "imgui/imgui.h"

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
	FWorld* World = GEngine->GetWorld();
	if (World == nullptr)
	{
		return;
	}

	if (abs(YOffset) >= DBL_EPSILON)
	{
		ACameraActor* Camera = World->GetCamera();
		if (Camera == nullptr)
		{
			return;
		}

		glm::mat4 RotationMatrix = glm::toMat4(Camera->GetRotation());

		glm::vec4 MoveVector(0.0f, 0.0f, -YOffset, 1.0f);

		float CameraMoveSpeed;
		GConfig->Get("CameraMoveSpeed", CameraMoveSpeed);

		Camera->SetLocation(Camera->GetLocation() + glm::vec3(RotationMatrix * MoveVector) * CameraMoveSpeed * 0.1f);
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

void Update(float InDeltaTime); 

int RandRange(int Min, int Max)
{
	return rand() % (Max - Min + 1) + Min;
}

class FMainWidget : public FWidget
{
public:
	FMainWidget();
	virtual ~FMainWidget() { }

	virtual void Draw();
	void OnShaderItemSelected(const std::string& NewSelectedItem);

private:
	bool bInitialized;
	std::vector<std::string> ShaderItems;
	std::string CurrentShaderItem = nullptr;

	glm::vec3 LightPosition;
	glm::vec4 Ambient;
	glm::vec4 Diffuse;
	glm::vec4 Specular;
	glm::vec4 Attenuation;
	float Shininess;
};

FMainWidget::FMainWidget()
	: bInitialized(false)
	, ShaderItems({ "phong", "blinn_phong" })
	, CurrentShaderItem(ShaderItems[1])
{
	LightPosition = glm::vec3(1.0f);
	Ambient = glm::vec4(0.05f, 0.05f, 0.05f, 1.0f);
	Diffuse = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
	Specular = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
	Attenuation = glm::vec4(1.0f, 0.1f, 0.1f, 1.0f);
	Shininess = 32;
}

void FMainWidget::Draw()
{
	ImGui::Begin("Properties");

	if (bInitialized == false)
	{
		ImGui::SetWindowPos(ImVec2(20, 20));
		ImGui::SetWindowSize(ImVec2(300, 300));
		bInitialized = true;
	}

	if (ImGui::BeginCombo("Shader", CurrentShaderItem.c_str()))
	{
		for (const std::string& Item : ShaderItems)
		{
			bool bIsSelected = CurrentShaderItem == Item;
			if (ImGui::Selectable(Item.c_str(), bIsSelected))
			{
				CurrentShaderItem = Item;
				OnShaderItemSelected(CurrentShaderItem.c_str());
			}

			if (bIsSelected)
			{
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}

	ImGui::InputFloat3("LightPosition", &LightPosition[0]);
	ImGui::SliderFloat4("Ambient", &Ambient[0], 0.0f, 1.0f);
	ImGui::SliderFloat4("Diffuse", &Diffuse[0], 0.0f, 1.0f);
	ImGui::SliderFloat4("Specular", &Specular[0], 0.0f, 1.0f);
	ImGui::SliderFloat4("Attenuation", &Attenuation[0], 0.0f, 1.0f);
	ImGui::SliderFloat("Shininess", &Shininess, 1.0f, 128.0f);

	FWorld* World = GEngine->GetWorld();
	if (World)
	{
		ALightActor* LightActor = World->GetLight();
		if (LightActor)
		{
			LightActor->SetLocation(LightPosition);
			LightActor->SetAmbient(Ambient);
			LightActor->SetDiffuse(Diffuse);
			LightActor->SetSpecular(Specular);
			LightActor->SetAttenuation(Ambient);
			LightActor->SetShininess(Shininess);
		}
	}

	ImGui::End();
}

FVulkanMeshRenderer* MeshRenderer;

void FMainWidget::OnShaderItemSelected(const std::string& NewSelectedItem)
{
	if (NewSelectedItem == "phong")
	{
		MeshRenderer->SetPipelineIndex(0);
	}
	else if (NewSelectedItem == "blinn_phong")
	{
		MeshRenderer->SetPipelineIndex(1);
	}
}

void Run(int argc, char** argv)
{
	srand(time(0));

	FConfig::Startup();

	std::string ProjectDirectory = SOLUTION_DIRECTORY PROJECT_NAME "/";

	GConfig->Set("ApplicationName", PROJECT_NAME);
	GConfig->Set("EngineName", "No Engine");
	GConfig->Set("WindowWidth", 800);
	GConfig->Set("WindowHeight", 600);
	GConfig->Set("WindowTitle", PROJECT_NAME);
	GConfig->Set("TargetFPS", 60.0f);
	GConfig->Set("MaxConcurrentFrames", 2);
	GConfig->Set("MouseSensitivity", 0.5f);
	GConfig->Set("CameraMoveSpeed", 1.0f);
	GConfig->Set("ShaderDirectory", ProjectDirectory + "Shaders/");
	GConfig->Set("ImageDirectory", ProjectDirectory + "Images/");
	GConfig->Set("ResourceDirectory", ProjectDirectory + "Resources/");

	FEngine::Init();

	for (const auto& Entry : std::filesystem::directory_iterator(ProjectDirectory + "Shaders"))
	{
		std::string Filename = Entry.path().string();
		std::string Extension = Entry.path().extension().string();
		if (Extension == ".vert" || Extension == ".frag")
		{
			std::string Command = "glslang -g -V ";
			Command += Filename;
			Command += " -o ";
			Command += Filename + ".spv";

			system(Command.c_str());
		}
	}

	GLFWwindow* Window = GEngine->GetWindow();

	glfwSetMouseButtonCallback(Window, OnMouseButtonEvent);
	glfwSetScrollCallback(Window, OnMouseWheelEvent);
	glfwSetKeyCallback(Window, OnKeyEvent);

	FVulkanUIRenderer* UIRenderer = GEngine->GetUIRenderer();
	UIRenderer->Ready();

	std::shared_ptr<FWidget> MainWidget = std::make_shared<FMainWidget>();
	UIRenderer->AddWidget(MainWidget);

	std::string ResourceDirectory;
	GConfig->Get("ResourceDirectory", ResourceDirectory);

	std::string ImageDirectory;
	GConfig->Get("ImageDirectory", ImageDirectory);

	FMesh* SphereMeshAsset = FAssetManager::CreateAsset<FMesh>();
	SphereMeshAsset->LoadObj(ResourceDirectory + "sphere.obj");

	FTextureSource* BrickBaseColorTextureSource = FAssetManager::CreateAsset<FTextureSource>();
	BrickBaseColorTextureSource->Load(ImageDirectory + "Brick_BaseColor.jpg");

	FTextureSource* BrickNormalTextureSource = FAssetManager::CreateAsset<FTextureSource>();
	BrickNormalTextureSource->Load(ImageDirectory + "Brick_Normal.png");

	FTextureSource* WhiteTextureSource = FAssetManager::CreateAsset<FTextureSource>();
	WhiteTextureSource->Load(ImageDirectory + "white.png");

	FWorld* World = GEngine->GetWorld();

	AMeshActor* LightSourceActor = World->SpawnActor<AMeshActor>();
	LightSourceActor->SetMeshAsset(SphereMeshAsset);
	LightSourceActor->SetBaseColorTexture(WhiteTextureSource);
	LightSourceActor->SetNormalTexture(BrickNormalTextureSource);
	LightSourceActor->SetScale(glm::vec3(0.1f, 0.1f, 0.1f));

	AMeshActor* SphereActor = World->SpawnActor<AMeshActor>();
	SphereActor->SetMeshAsset(SphereMeshAsset);
	SphereActor->SetBaseColorTexture(BrickBaseColorTextureSource);
	SphereActor->SetNormalTexture(BrickNormalTextureSource);
	SphereActor->SetLocation(glm::vec3(0.0f, 0.0f, -2.0f));

	FVulkanContext* RenderContext = GEngine->GetRenderContext();
	MeshRenderer = RenderContext->CreateObject<FVulkanMeshRenderer>();
	MeshRenderer->SetPipelineIndex(1);

	float TargetFPS;
	GConfig->Get("TargetFPS", TargetFPS);

	clock_t PreviousFrameTime = clock();
	float MaxFrameTime = 1000.0f / TargetFPS;

	float TotalFrameTime = 0.0f;
	int TotalFrameCount = 0;

	while (!glfwWindowShouldClose(Window))
	{
		clock_t CurrentFrameTime = clock();
		float DeltaTime = static_cast<float>(CurrentFrameTime - PreviousFrameTime) / CLOCKS_PER_SEC;

		glfwPollEvents();
		Update(DeltaTime);

		GEngine->Tick(DeltaTime);

		MeshRenderer->PreRender();

		RenderContext->BeginRender();
		MeshRenderer->Render();
		UIRenderer->Render();
		RenderContext->EndRender();

		PreviousFrameTime = CurrentFrameTime;

		TotalFrameTime += DeltaTime;
		++TotalFrameCount;

		if (TotalFrameTime >= 5.0f)
		{
			std::cout << "Average Frame: " << TotalFrameCount / TotalFrameTime << std::endl;
			TotalFrameTime = 0.0f;
			TotalFrameCount = 0;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds((int)(MaxFrameTime)));
	}

	MeshRenderer->WaitIdle();

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

	FWorld* World = GEngine->GetWorld();
	if (World == nullptr)
	{
		return;
	}

	glm::vec3 RotationDelta(0.0f);

	double MouseX, MouseY;
	glfwGetCursorPos(Window, &MouseX, &MouseY);

	ACameraActor* Camera = World->GetCamera();
	if (Camera)
	{
		double MouseDeltaX = MouseX - PrevMouseX;
		double MouseDeltaY = MouseY - PrevMouseY;

		float MouseSensitivity;
		GConfig->Get("MouseSensitivity", MouseSensitivity);

		float PitchAmount = MouseDeltaY * MouseSensitivity * InDeltaTime;
		float YawAmount = -MouseDeltaX * MouseSensitivity * InDeltaTime;

		if (abs(PitchAmount) > FLT_EPSILON || abs(YawAmount) > FLT_EPSILON)
		{
			glm::quat PitchRotation = glm::angleAxis(PitchAmount, glm::vec3(1.0f, 0.0f, 0.0f));
			glm::quat YawRotation = glm::angleAxis(YawAmount, glm::vec3(0.0f, 1.0f, 0.0f));

			Camera->SetRotation(YawRotation * Camera->GetRotation() * PitchRotation);
		}

		PrevMouseX = MouseX;
		PrevMouseY = MouseY;

		glm::mat4 RotationMatrix = glm::toMat4(Camera->GetRotation());

		glm::vec4 MoveVector = RotationMatrix * glm::vec4(glm::normalize(CameraMoveDelta), 1.0f);
		glm::vec3 FinalMoveDelta(MoveVector);
		FinalMoveDelta = glm::normalize(FinalMoveDelta);

		float CameraMoveSpeed;
		GConfig->Get("CameraMoveSpeed", CameraMoveSpeed);

		if (glm::length(FinalMoveDelta) > FLT_EPSILON)
		{
			Camera->SetLocation(Camera->GetLocation() + FinalMoveDelta * CameraMoveSpeed * InDeltaTime);
		}
	}
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

