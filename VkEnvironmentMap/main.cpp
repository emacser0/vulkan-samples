#include "Config.h"
#include "Mesh.h"
#include "AssetManager.h"
#include "Texture2D.h"
#include "TextureCube.h"
#include "Widget.h"

#include "VulkanContext.h"
#include "VulkanModel.h"
#include "VulkanScene.h"
#include "VulkanMeshRenderer.h"
#include "VulkanSkyRenderer.h"
#include "VulkanUIRenderer.h"

#include "Engine.h"
#include "World.h"
#include "Actor.h"
#include "MeshActor.h"
#include "SkyActor.h"

#include <ctime>
#include <chrono>
#include <thread>
#include <iostream>
#include <stdexcept>
#include <cstdlib>

#include "glm/glm.hpp"

#include "imgui/imgui.h"

FVulkanSkyRenderer* SkyRenderer;

class FMainWidget : public FWidget
{
public:
	FMainWidget();
	virtual ~FMainWidget() { }

	virtual void Draw();

private:
	bool bInitialized;

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

	ImGui::Checkbox("Show TBN", &bShowTBN);
	ImGui::Checkbox("Light Attenuation", &bEnableAttenuation);
	ImGui::Checkbox("Gamma Correction", &bEnableGammaCorrection);
	ImGui::Checkbox("Tone Mapping", &bEnableToneMapping);

	FVulkanMeshRenderer* MeshRenderer = GEngine->GetMeshRenderer();
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

	UTexture2D* PlaneNormalTexture = FAssetManager::CreateAsset<UTexture2D>("T_PlaneNormal");
	PlaneNormalTexture->Load(ImageDirectory + "normal.png", true);

	UTextureCube* SkyTexture = FAssetManager::CreateAsset<UTextureCube>("T_Sky");
	{
		std::array<std::string, 6> SkyTextureFilenames;
		for (int Idx = 0; Idx < SkyTextureFilenames.size(); ++Idx)
		{
			SkyTextureFilenames[Idx] = ImageDirectory + "Skybox_" + std::string(1, '0' + Idx) + ".jpg";
		}
		SkyTexture->Load(SkyTextureFilenames);
	}

	std::string ShaderDirectory;
	GConfig->Get("ShaderDirectory", ShaderDirectory);

	UMaterial* BaseMaterial = FAssetManager::CreateAsset<UMaterial>("M_Base");
	{
		FShaderPath ShaderPath{};
		ShaderPath.VS = ShaderDirectory + "base.vert.spv";
		ShaderPath.FS = ShaderDirectory + "base.frag.spv";
		BaseMaterial->SetShaderPath(ShaderPath);

		FShaderParameter BaseColorParameter{};
		BaseColorParameter.Type = EShaderParameterType::Texture;
		BaseColorParameter.TexParam = SkyTexture;
		BaseMaterial->SetBaseColor(BaseColorParameter);

		FShaderParameter NormalParameter{};
		NormalParameter.Type = EShaderParameterType::Texture;
		NormalParameter.TexParam = PlaneNormalTexture;
		BaseMaterial->SetNormal(NormalParameter);

		BaseMaterial->CreateRenderMaterial();
	}

	UMaterial* SkyMaterial = FAssetManager::CreateAsset<UMaterial>("M_Sky");
	{
		FShaderPath ShaderPath{};
		ShaderPath.VS = ShaderDirectory + "sky.vert.spv";
		ShaderPath.FS = ShaderDirectory + "sky.frag.spv";
		SkyMaterial->SetShaderPath(ShaderPath);

		FShaderParameter BaseColorParameter{};
		BaseColorParameter.Type = EShaderParameterType::Texture;
		BaseColorParameter.TexParam = SkyTexture;
		SkyMaterial->SetBaseColor(BaseColorParameter);

		FShaderParameter NormalParameter{};
		NormalParameter.Type = EShaderParameterType::Texture;
		NormalParameter.TexParam = PlaneNormalTexture;
		SkyMaterial->SetNormal(NormalParameter);

		SkyMaterial->CreateRenderMaterial();
	}

	UMesh* SphereMesh = FAssetManager::CreateAsset<UMesh>("SM_Sphere");
	SphereMesh->Load(MeshDirectory + "sphere.fbx");
	SphereMesh->SetMaterial(BaseMaterial);

	UMesh* SkyMesh = FAssetManager::CreateAsset<UMesh>("SM_Sky");
	SkyMesh->Load(MeshDirectory + "sphere.fbx");
	SkyMesh->SetMaterial(SkyMaterial);

	FWorld* World = GEngine->GetWorld();

	AMeshActor* SphereActor = World->SpawnActor<AMeshActor>();
	SphereActor->SetMesh(SphereMesh);
	SphereActor->SetLocation(glm::vec3(0.0f, 0.0f, -2.0f));
	SphereActor->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));

	ASkyActor* SkyActor = World->GetSky();
	SkyActor->SetMesh(SkyMesh);

	FVulkanContext* RenderContext = GEngine->GetRenderContext();
	FVulkanMeshRenderer* MeshRenderer = GEngine->GetMeshRenderer();

	SkyRenderer = RenderContext->CreateObject<FVulkanSkyRenderer>();

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

		SkyRenderer->PreRender();
		MeshRenderer->PreRender();

		RenderContext->BeginRender();
		SkyRenderer->Render();
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

