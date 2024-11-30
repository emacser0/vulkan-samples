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
#include "LightActor.h"
#include "PointLightActor.h"
#include "DirectionalLightActor.h"
#include "MeshActor.h"

#include <ctime>
#include <chrono>
#include <thread>
#include <iostream>
#include <stdexcept>
#include <cstdlib>

#include "glm/glm.hpp"

#include "imgui/imgui.h"

int RandRange(int Min, int Max)
{
	return rand() % (Max - Min + 1) + Min;
}

FVulkanMeshRenderer* MeshRenderer;

APointLightActor* PointLight;

class FMainWidget : public FWidget
{
public:
	FMainWidget();
	virtual ~FMainWidget() { }

	virtual void Draw();

private:
	bool bInitialized;

	glm::vec3 LightPosition;
	glm::vec4 Ambient;
	glm::vec4 Diffuse;
	glm::vec4 Specular;
	glm::vec4 Attenuation;
	float Shininess;

	bool bShowTBN;
	bool bEnableAttenuation;
	bool bEnableGammaCorrection;
	bool bEnableToneMapping;
};

FMainWidget::FMainWidget()
	: bInitialized(false)
	, bShowTBN(false)
	, bEnableAttenuation(false)
	, bEnableGammaCorrection(false)
	, bEnableToneMapping(false)
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

	ImGui::BeginGroup();

	ImGui::InputFloat3("LightPosition", &LightPosition[0]);
	ImGui::SliderFloat4("Ambient", &Ambient[0], 0.0f, 1.0f);
	ImGui::SliderFloat4("Diffuse", &Diffuse[0], 0.0f, 1.0f);
	ImGui::SliderFloat4("Specular", &Specular[0], 0.0f, 1.0f);
	ImGui::SliderFloat4("Attenuation", &Attenuation[0], 0.0f, 1.0f);
	ImGui::SliderFloat("Shininess", &Shininess, 1.0f, 128.0f);

	ImGui::EndGroup();

	if (PointLight != nullptr)
	{
		PointLight->SetLocation(LightPosition);
		PointLight->SetAmbient(Ambient);
		PointLight->SetDiffuse(Diffuse);
		PointLight->SetSpecular(Specular);
		PointLight->SetAttenuation(Attenuation);
		PointLight->SetShininess(Shininess);
	}

	ImGui::Checkbox("Show TBN", &bShowTBN);
	ImGui::Checkbox("Light Attenuation", &bEnableAttenuation);
	ImGui::Checkbox("Gamma Correction", &bEnableGammaCorrection);
	ImGui::Checkbox("Tone Mapping", &bEnableToneMapping);
	if (MeshRenderer != nullptr)
	{
		MeshRenderer->SetEnableTBNVisualization(bShowTBN);
		MeshRenderer->SetEnableAttenuation(bEnableAttenuation);
		MeshRenderer->SetEnableGammaCorrection(bEnableGammaCorrection);
		MeshRenderer->SetEnableToneMapping(bEnableToneMapping);
	}

	ImGui::End();
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

	FVulkanUIRenderer* UIRenderer = GEngine->GetUIRenderer();
	UIRenderer->Ready();

	std::shared_ptr<FWidget> MainWidget = std::make_shared<FMainWidget>();
	UIRenderer->AddWidget(MainWidget);

	std::string MeshDirectory;
	GConfig->Get("MeshDirectory", MeshDirectory);

	std::string ImageDirectory;
	GConfig->Get("ImageDirectory", ImageDirectory);

	FMesh* SphereMeshAsset = FAssetManager::CreateAsset<FMesh>();
	SphereMeshAsset->Load(MeshDirectory + "sphere.fbx");

	FTextureSource* BrickBaseColorTextureSource = FAssetManager::CreateAsset<FTextureSource>();
	BrickBaseColorTextureSource->Load(ImageDirectory + "Brick_BaseColor.jpg");

	FTextureSource* BrickNormalTextureSource = FAssetManager::CreateAsset<FTextureSource>();
	BrickNormalTextureSource->Load(ImageDirectory + "Brick_Normal.png");

	FTextureSource* WhiteTextureSource = FAssetManager::CreateAsset<FTextureSource>();
	WhiteTextureSource->Load(ImageDirectory + "white.png");

	FWorld* World = GEngine->GetWorld();

	PointLight = World->SpawnActor<APointLightActor>();

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
	SphereActor->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));

	AMeshActor* SphereActor2 = World->SpawnActor<AMeshActor>();
	SphereActor2->SetMeshAsset(SphereMeshAsset);
	SphereActor2->SetBaseColorTexture(BrickBaseColorTextureSource);
	SphereActor2->SetNormalTexture(BrickNormalTextureSource);
	SphereActor2->SetLocation(glm::vec3(4.0f, 0.0f, -2.0f));
	SphereActor2->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));

	FVulkanContext* RenderContext = GEngine->GetRenderContext();
	MeshRenderer = RenderContext->CreateObject<FVulkanMeshRenderer>();

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

	RenderContext->WaitIdle();

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

