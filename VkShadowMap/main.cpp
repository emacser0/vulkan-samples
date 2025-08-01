#include "Config.h"
#include "Mesh.h"
#include "Material.h"
#include "AssetManager.h"
#include "Texture2D.h"
#include "TextureCube.h"
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
#include "PointLightActor.h"
#include "DirectionalLightActor.h"
#include "MeshActor.h"
#include "SkyActor.h"

#include <ctime>
#include <chrono>
#include <thread>
#include <iostream>
#include <stdexcept>
#include <cstdlib>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/quaternion.hpp"

#include "imgui/imgui.h"

APointLightActor* PointLight;
ADirectionalLightActor* DirectionalLight;

AMeshActor* LightSourceActor;

class FMainWidget : public FWidget
{
public:
	FMainWidget();
	virtual ~FMainWidget() { }

	virtual void Draw();

private:
	bool bInitialized;

	glm::vec3 PointLightPosition;
	glm::vec4 PointAmbient;
	glm::vec4 PointDiffuse;
	glm::vec4 PointSpecular;
	glm::vec4 PointAttenuation;
	float PointShininess;

	glm::vec3 DirDirection;
	glm::vec4 DirAmbient;
	glm::vec4 DirDiffuse;
	glm::vec4 DirSpecular;
	glm::vec4 DirAttenuation;
	float DirShininess;

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
	PointLightPosition = glm::vec3(1.0f, -0.5f, 1.0f);
	PointAmbient = glm::vec4(0.05f, 0.05f, 0.05f, 1.0f);
	PointDiffuse = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
	PointSpecular = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
	PointAttenuation = glm::vec4(1.0f, 0.1f, 0.1f, 1.0f);
	PointShininess = 32;

	DirDirection = glm::vec3(0.0f, -1.0f, 0.0f);
	DirAmbient = glm::vec4(0.05f, 0.05f, 0.05f, 1.0f);
	DirDiffuse = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
	DirSpecular = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
	DirAttenuation = glm::vec4(1.0f, 0.1f, 0.1f, 1.0f);
	DirShininess = 32;
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

	ImGui::Text("Point Light");

	ImGui::InputFloat3("Point Position", &PointLightPosition[0]);
	ImGui::SliderFloat4("Point Ambient", &PointAmbient[0], 0.0f, 1.0f);
	ImGui::SliderFloat4("Point Diffuse", &PointDiffuse[0], 0.0f, 1.0f);
	ImGui::SliderFloat4("Point Specular", &PointSpecular[0], 0.0f, 1.0f);
	ImGui::SliderFloat4("Point Attenuation", &PointAttenuation[0], 0.0f, 1.0f);
	ImGui::SliderFloat("Point Shininess", &PointShininess, 1.0f, 128.0f);

	if (PointLight != nullptr)
	{
		PointLight->SetLocation(PointLightPosition);
		PointLight->SetAmbient(PointAmbient);
		PointLight->SetDiffuse(PointDiffuse);
		PointLight->SetSpecular(PointSpecular);
		PointLight->SetAttenuation(PointAttenuation);
		PointLight->SetShininess(PointShininess);
	}

	if (LightSourceActor != nullptr)
	{
		LightSourceActor->SetLocation(PointLightPosition);
	}

	ImGui::Text("Directional Light");

	ImGui::InputFloat3("Directional Direction", &DirDirection[0]);
	ImGui::SliderFloat4("Directional Ambient", &DirAmbient[0], 0.0f, 1.0f);
	ImGui::SliderFloat4("Directional Diffuse", &DirDiffuse[0], 0.0f, 1.0f);
	ImGui::SliderFloat4("Directional Specular", &DirSpecular[0], 0.0f, 1.0f);
	ImGui::SliderFloat4("Directional Attenuation", &DirAttenuation[0], 0.0f, 1.0f);
	ImGui::SliderFloat("Directional Shininess", &DirShininess, 1.0f, 128.0f);

	if (DirectionalLight != nullptr)
	{
		DirectionalLight->SetDirection(DirDirection);
		DirectionalLight->SetAmbient(DirAmbient);
		DirectionalLight->SetDiffuse(DirDiffuse);
		DirectionalLight->SetSpecular(DirSpecular);
		DirectionalLight->SetAttenuation(DirAttenuation);
		DirectionalLight->SetShininess(DirShininess);
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
	srand(static_cast<unsigned int>(time(NULL)));

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

	GLFWwindow* Window = GEngine->GetWindow();

	FVulkanContext* RenderContext = GEngine->GetRenderContext();
	FVulkanUIRenderer* UIRenderer = RenderContext->GetUIRenderer();

	std::shared_ptr<FWidget> MainWidget = std::make_shared<FMainWidget>();
	UIRenderer->AddWidget(MainWidget);

	std::string MeshDirectory;
	GConfig->Get("MeshDirectory", MeshDirectory);

	std::string ImageDirectory;
	GConfig->Get("ImageDirectory", ImageDirectory);

	UTexture2D* BrickBaseColorTexture = FAssetManager::CreateAsset<UTexture2D>("T_BrickBaseColor");
	BrickBaseColorTexture->Load(ImageDirectory + "Brick_BaseColor.jpg");

	UTexture2D* BrickNormalTexture = FAssetManager::CreateAsset<UTexture2D>("T_BrickNormal");
	BrickNormalTexture->Load(ImageDirectory + "Brick_Normal.png", true);

	UTexture2D* WhiteTexture = FAssetManager::CreateAsset<UTexture2D>("T_White");
	WhiteTexture->Load(ImageDirectory + "white.png");

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
		BaseColorParameter.TexParam = BrickBaseColorTexture;
		BaseMaterial->SetBaseColor(BaseColorParameter);

		FShaderParameter NormalParameter{};
		NormalParameter.Type = EShaderParameterType::Texture;
		NormalParameter.TexParam = BrickNormalTexture;
		BaseMaterial->SetNormal(NormalParameter);

		FShaderParameter AmbientParameter{};
		AmbientParameter.Type = EShaderParameterType::Vector3;
		AmbientParameter.Vec3Param = glm::vec3(0.05f, 0.05f, 0.05f);
		BaseMaterial->SetAmbient(AmbientParameter);

		FShaderParameter DiffuseParameter{};
		DiffuseParameter.Type = EShaderParameterType::Vector3;
		DiffuseParameter.Vec3Param = glm::vec3(1.0f);
		BaseMaterial->SetDiffuse(DiffuseParameter);

		FShaderParameter SpecularParameter{};
		SpecularParameter.Type = EShaderParameterType::Vector3;
		SpecularParameter.Vec3Param = glm::vec3(1.0f);
		BaseMaterial->SetSpecular(SpecularParameter);

		BaseMaterial->CreateRenderMaterial();
	}

	UMaterial* BaseplateMaterial = FAssetManager::CreateAsset<UMaterial>("M_Baseplate");
	{
		FShaderPath ShaderPath{};
		ShaderPath.VS = ShaderDirectory + "base.vert.spv";
		ShaderPath.FS = ShaderDirectory + "base.frag.spv";
		BaseplateMaterial->SetShaderPath(ShaderPath);

		FShaderParameter BaseColorParameter{};
		BaseColorParameter.Type = EShaderParameterType::Texture;
		BaseColorParameter.TexParam = WhiteTexture;
		BaseplateMaterial->SetBaseColor(BaseColorParameter);

		FShaderParameter NormalParameter{};
		NormalParameter.Type = EShaderParameterType::Texture;
		NormalParameter.TexParam = PlaneNormalTexture;
		BaseplateMaterial->SetNormal(NormalParameter);

		FShaderParameter AmbientParameter{};
		AmbientParameter.Type = EShaderParameterType::Vector3;
		AmbientParameter.Vec3Param = glm::vec3(0.05f, 0.05f, 0.05f);
		BaseplateMaterial->SetAmbient(AmbientParameter);

		FShaderParameter DiffuseParameter{};
		DiffuseParameter.Type = EShaderParameterType::Vector3;
		DiffuseParameter.Vec3Param = glm::vec3(1.0f);
		BaseplateMaterial->SetDiffuse(DiffuseParameter);

		FShaderParameter SpecularParameter{};
		SpecularParameter.Type = EShaderParameterType::Vector3;
		SpecularParameter.Vec3Param = glm::vec3(0.0f);
		BaseplateMaterial->SetSpecular(SpecularParameter);

		BaseplateMaterial->CreateRenderMaterial();
	}

	UMaterial* LightSourceMaterial = FAssetManager::CreateAsset<UMaterial>("M_LightSource");
	{
		FShaderPath ShaderPath{};
		ShaderPath.VS = ShaderDirectory + "lightSource.vert.spv";
		ShaderPath.FS = ShaderDirectory + "lightSource.frag.spv";
		LightSourceMaterial->SetShaderPath(ShaderPath);

		FShaderParameter BaseColorParameter{};
		BaseColorParameter.Type = EShaderParameterType::Texture;
		BaseColorParameter.TexParam = WhiteTexture;
		LightSourceMaterial->SetBaseColor(BaseColorParameter);

		FShaderParameter NormalParameter{};
		NormalParameter.Type = EShaderParameterType::Texture;
		NormalParameter.TexParam = PlaneNormalTexture;
		LightSourceMaterial->SetNormal(NormalParameter);

		FShaderParameter AmbientParameter{};
		AmbientParameter.Type = EShaderParameterType::Vector3;
		AmbientParameter.Vec3Param = glm::vec3(0.05f, 0.05f, 0.05f);
		LightSourceMaterial->SetAmbient(AmbientParameter);

		FShaderParameter DiffuseParameter{};
		DiffuseParameter.Type = EShaderParameterType::Vector3;
		DiffuseParameter.Vec3Param = glm::vec3(1.0f);
		LightSourceMaterial->SetDiffuse(DiffuseParameter);

		FShaderParameter SpecularParameter{};
		SpecularParameter.Type = EShaderParameterType::Vector3;
		SpecularParameter.Vec3Param = glm::vec3(1.0f);
		LightSourceMaterial->SetSpecular(SpecularParameter);

		LightSourceMaterial->CreateRenderMaterial();
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

	UMesh* LightSourceMesh = FAssetManager::CreateAsset<UMesh>("SM_LightSource");
	LightSourceMesh->Load(MeshDirectory + "sphere.fbx");
	LightSourceMesh->SetMaterial(LightSourceMaterial);

	UMesh* SkyMesh = FAssetManager::CreateAsset<UMesh>("SM_Sky");
	SkyMesh->Load(MeshDirectory + "sphere.fbx");
	SkyMesh->SetMaterial(SkyMaterial);

	UMesh* BaseplateMesh = FAssetManager::CreateAsset<UMesh>("SM_Baseplate");
	BaseplateMesh->Load(MeshDirectory + "cube.obj");
	BaseplateMesh->SetMaterial(BaseplateMaterial);

	FWorld* World = GEngine->GetWorld();

	PointLight = World->SpawnActor<APointLightActor>();
	DirectionalLight = World->SpawnActor<ADirectionalLightActor>();

	LightSourceActor = World->SpawnActor<AMeshActor>();
	LightSourceActor->SetMesh(LightSourceMesh);
	LightSourceActor->SetScale(glm::vec3(0.1f, 0.1f, 0.1f));

	AMeshActor* SphereActor = World->SpawnActor<AMeshActor>();
	SphereActor->SetMesh(SphereMesh);
	SphereActor->SetLocation(glm::vec3(0.0f, 0.0f, -2.0f));
	SphereActor->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));

	AMeshActor* SphereActor2 = World->SpawnActor<AMeshActor>();
	SphereActor2->SetMesh(SphereMesh);
	SphereActor2->SetLocation(glm::vec3(4.0f, 0.0f, -2.0f));
	SphereActor2->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));

	AMeshActor* BaseplateActor = World->SpawnActor<AMeshActor>();
	BaseplateActor->SetMesh(BaseplateMesh);
	BaseplateActor->SetLocation(glm::vec3(0.0f, 1.0f, 0.0f));
	BaseplateActor->SetScale(glm::vec3(15.0f, 0.1f, 15.0f));

	ASkyActor* SkyActor = World->GetSky();
	SkyActor->SetMesh(SkyMesh);

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

