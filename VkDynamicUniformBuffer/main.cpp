#include "Config.h"
#include "AssetManager.h"
#include "Mesh.h"
#include "TextureSource.h"
#include "Widget.h"

#include "VulkanContext.h"
#include "VulkanModel.h"
#include "VulkanScene.h"
#include "VulkanMeshRenderer.h"
#include "VulkanUIRenderer.h"

#include "Camera.h"
#include "Engine.h"

#include <ctime>
#include <chrono>
#include <thread>
#include <iostream>
#include <stdexcept>

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

	FVulkanLight Light;
};

FMainWidget::FMainWidget()
	: bInitialized(false)
	, ShaderItems({ "vert_phong", "frag_phong", "blinn_phong" })
	, CurrentShaderItem(ShaderItems[0])
{
	Light.Position = glm::vec3(0.0f);
	Light.Ambient = glm::vec4(0.05f, 0.05f, 0.05f, 1.0f);
	Light.Diffuse = glm::vec4(0.5f, 0.5f, 0.5f, 0.5f);
	Light.Specular = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
	Light.Attenuation = glm::vec4(1.0f, 0.1f, 0.1f, 1.0f);
	Light.Shininess = 32;
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

	ImGui::InputFloat3("LightPosition", &Light.Position[0]);
	ImGui::SliderFloat4("Ambient", &Light.Ambient[0], 0.0f, 1.0f);
	ImGui::SliderFloat4("Diffuse", &Light.Diffuse[0], 0.0f, 1.0f);
	ImGui::SliderFloat4("Specular", &Light.Specular[0], 0.0f, 1.0f);
	ImGui::SliderFloat4("Attenuation", &Light.Attenuation[0], 0.0f, 1.0f);
	ImGui::SliderFloat("Shininess", &Light.Shininess, 1.0f, 128.0f);

	FVulkanScene* Scene = GEngine->GetScene();
	if (Scene)
	{
		Scene->SetLight(Light);
	}

	ImGui::End();
}

FVulkanMeshRenderer* MeshRenderer;

void FMainWidget::OnShaderItemSelected(const std::string& NewSelectedItem)
{
	if (NewSelectedItem == "vert_phong")
	{
		MeshRenderer->SetPipelineIndex(0);
	}
	else if (NewSelectedItem == "frag_phong")
	{
		MeshRenderer->SetPipelineIndex(1);
	}
	else if (NewSelectedItem == "blinn_phong")
	{
		MeshRenderer->SetPipelineIndex(2);
	}
}

void Run(int argc, char** argv)
{
	srand(time(0));

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

	GLFWwindow* Window = GEngine->GetWindow();

	glfwSetMouseButtonCallback(Window, OnMouseButtonEvent);
	glfwSetScrollCallback(Window, OnMouseWheelEvent);
	glfwSetKeyCallback(Window, OnKeyEvent);

	FVulkanUIRenderer* UIRenderer = GEngine->GetUIRenderer();
	UIRenderer->Ready();

	std::shared_ptr<FWidget> MainWidget = std::make_shared<FMainWidget>();
	UIRenderer->AddWidget(MainWidget);

	std::string MeshDirectory;
	GConfig->Get("MeshDirectory", MeshDirectory);

	std::string ImageDirectory;
	GConfig->Get("ImageDirectory", ImageDirectory);

	FVulkanContext* RenderContext = GEngine->GetRenderContext();

	FMesh* SphereMeshAsset = FAssetManager::CreateAsset<FMesh>();
	SphereMeshAsset->Load(MeshDirectory + "sphere.obj");

	FMesh* MonkeyMeshAsset = FAssetManager::CreateAsset<FMesh>();
	MonkeyMeshAsset->Load(MeshDirectory + "monkey.obj");

	std::vector<FTextureSource*> TextureSources;
	{
		FTextureSource* NewTextureSource = FAssetManager::CreateAsset<FTextureSource>();
		NewTextureSource->Load(ImageDirectory + "purple.png");
		TextureSources.push_back(NewTextureSource);
	}
	{
		FTextureSource* NewTextureSource = FAssetManager::CreateAsset<FTextureSource>();
		NewTextureSource->Load(ImageDirectory + "orange.png");
		TextureSources.push_back(NewTextureSource);
	}
	{
		FTextureSource* NewTextureSource = FAssetManager::CreateAsset<FTextureSource>();
		NewTextureSource->Load(ImageDirectory + "green.png");
		TextureSources.push_back(NewTextureSource);
	}

	FTextureSource* WhiteTextureSource = FAssetManager::CreateAsset<FTextureSource>();
	WhiteTextureSource->Load(ImageDirectory + "white.png");

	std::vector<FVulkanTexture*> Textures;
	for (const auto& TextureSource : TextureSources)
	{
		FVulkanTexture* Texture = RenderContext->CreateObject<FVulkanTexture>();
		Texture->LoadSource(*TextureSource);
		Textures.push_back(Texture);
	}

	FVulkanTexture* WhiteTexture = new FVulkanTexture(RenderContext);
	WhiteTexture->LoadSource(*WhiteTextureSource);

	std::vector<FVulkanMesh*> Meshes;
	for (const auto& Texture : Textures)
	{
		FVulkanMesh* Mesh = RenderContext->CreateObject<FVulkanMesh>();
		Mesh->Load(SphereMeshAsset);
		Mesh->SetTexture(Texture);
		Meshes.push_back(Mesh);
	}

	FVulkanMesh* MonkeyMesh = RenderContext->CreateObject<FVulkanMesh>();
	MonkeyMesh->Load(MonkeyMeshAsset);
	MonkeyMesh->SetTexture(WhiteTexture);

	FVulkanModel* MonkeyModel = RenderContext->CreateObject<FVulkanModel>();
	MonkeyModel->SetMesh(MonkeyMesh);

	FTransform MonkeyTransform = MonkeyModel->GetTransform();
	MonkeyTransform.SetTranslation(glm::vec3(0.0f, 0.0f, 0.0f));
	MonkeyModel->SetTransform(MonkeyTransform);
	GEngine->GetScene()->AddModel(MonkeyModel);

	for (int32_t Idx = 0; Idx < 10; ++Idx)
	{
		FVulkanModel* Model = RenderContext->CreateObject<FVulkanModel>();
		Model->SetMesh(Meshes[RandRange(0, Meshes.size() - 1)]);

		FTransform ModelTransform = Model->GetTransform();
		ModelTransform.SetTranslation(glm::vec3(RandRange(-10, 10), RandRange(-10, 10), RandRange(-10, 10)));
		ModelTransform.SetScale(glm::vec3(1.0f, 1.0f, 1.0f));

		Model->SetTransform(ModelTransform);

		GEngine->GetScene()->AddModel(Model);
	}

	MeshRenderer = RenderContext->CreateObject<FVulkanMeshRenderer>();
	MeshRenderer->Ready();
	MeshRenderer->SetPipelineIndex(0);

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

