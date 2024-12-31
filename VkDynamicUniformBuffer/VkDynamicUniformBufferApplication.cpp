#include "VkDynamicUniformBufferApplication.h"
#include "Engine.h"
#include "Config.h"
#include "Utils.h"
#include "AssetManager.h"
#include "Texture.h"
#include "Mesh.h"
#include "Widget.h"
#include "Camera.h"
#include "CameraController.h"

#include "VulkanContext.h"
#include "VulkanMeshRenderer.h"
#include "VulkanUIRenderer.h"
#include "VulkanContext.h"
#include "VulkanMesh.h"
#include "VulkanTexture.h"
#include "VulkanScene.h"
#include "VulkanModel.h"

#include "imgui/imgui.h"

#include <ctime>

class FMainWidget : public FWidget
{
public:
	FMainWidget(FVulkanMeshRenderer* InMeshRenderer);
	virtual ~FMainWidget() { }

	virtual void Draw();
	void OnShaderItemSelected(const std::string& NewSelectedItem);

private:
	FVulkanMeshRenderer* MeshRenderer;

	bool bInitialized;
	std::vector<std::string> ShaderItems;
	std::string CurrentShaderItem = nullptr;

	FVulkanLight Light;
};

FMainWidget::FMainWidget(FVulkanMeshRenderer* InMeshRenderer)
	: MeshRenderer(InMeshRenderer)
	, bInitialized(false)
	, ShaderItems({ "vert_phong", "frag_phong", "blinn_phong" })
	, CurrentShaderItem(ShaderItems[0])
{
	Light.Position = glm::vec3(0.0f);
	Light.Ambient = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
	Light.Diffuse = glm::vec4(0.5f, 0.5f, 0.5f, 0.5f);
	Light.Specular = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
	Light.Attenuation = glm::vec4(1.0f, 0.2f, 0.2f, 1.0f);
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

FVkDynamicUniformBufferApplication::FVkDynamicUniformBufferApplication()
	: Camera(nullptr)
	, CameraController(nullptr)
	, MeshRenderer(nullptr)
	, Mesh(nullptr)
	, Texture(nullptr)
{
	srand(static_cast<unsigned int>(time(NULL)));

}

void FVkDynamicUniformBufferApplication::Run()
{
	GConfig->Set("CameraMoveSpeed", 1.0f);
	GConfig->Set("MouseSensitivity", 1.0f);

	Camera = std::make_shared<FCamera>();
	CameraController = std::make_shared<FCameraController>(Camera);

	std::string MeshDirectory;
	GConfig->Get("MeshDirectory", MeshDirectory);

	std::string ImageDirectory;
	GConfig->Get("ImageDirectory", ImageDirectory);

	FMesh* SphereMeshAsset = FAssetManager::CreateAsset<FMesh>();
	SphereMeshAsset->Load(MeshDirectory + "sphere.obj");

	FMesh* MonkeyMeshAsset = FAssetManager::CreateAsset<FMesh>();
	MonkeyMeshAsset->Load(MeshDirectory + "monkey.obj");

	std::vector<FTexture*> TextureSources;
	{
		FTexture* NewTextureSource = FAssetManager::CreateAsset<FTexture>();
		NewTextureSource->Load(ImageDirectory + "purple.png");
		TextureSources.push_back(NewTextureSource);
	}
	{
		FTexture* NewTextureSource = FAssetManager::CreateAsset<FTexture>();
		NewTextureSource->Load(ImageDirectory + "orange.png");
		TextureSources.push_back(NewTextureSource);
	}
	{
		FTexture* NewTextureSource = FAssetManager::CreateAsset<FTexture>();
		NewTextureSource->Load(ImageDirectory + "green.png");
		TextureSources.push_back(NewTextureSource);
	}

	FTexture* WhiteTextureSource = FAssetManager::CreateAsset<FTexture>();
	WhiteTextureSource->Load(ImageDirectory + "white.png");

	FVulkanContext* RenderContext = GEngine->GetRenderContext();

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
	MeshRenderer->SetPipelineIndex(0);

	std::shared_ptr<FWidget> MainWidget = std::make_shared<FMainWidget>(MeshRenderer);
	GEngine->AddWidget(MainWidget);
}

void FVkDynamicUniformBufferApplication::Terminate()
{
	FVulkanContext* RenderContext = GEngine->GetRenderContext();
	RenderContext->WaitIdle();
	RenderContext->DestroyObject(MeshRenderer);
}

void FVkDynamicUniformBufferApplication::Tick(float InDeltaTime)
{
	CameraController->Tick(InDeltaTime);

	MeshRenderer->SetViewMatrix(Camera->GetViewMatrix());
	MeshRenderer->SetProjectionMatrix(Camera->GetProjectionMatrix());
	MeshRenderer->SetCameraPosition(Camera->GetLocation());

	FVulkanContext* RenderContext = GEngine->GetRenderContext();
	FVulkanUIRenderer* UIRenderer = GEngine->GetUIRenderer();

	RenderContext->BeginRender();
	MeshRenderer->Render();
	UIRenderer->Render(GEngine->GetWidgets());
	RenderContext->EndRender();
}

void FVkDynamicUniformBufferApplication::OnMouseButtonDown(int InButton, int InMods)
{
	CameraController->OnMouseButtonDown(InButton, InMods);
}

void FVkDynamicUniformBufferApplication::OnMouseButtonUp(int InButton, int InMods)
{
	CameraController->OnMouseButtonUp(InButton, InMods);
}

void FVkDynamicUniformBufferApplication::OnMouseWheel(double InXOffset, double InYOffset)
{
	CameraController->OnMouseWheel(InXOffset, InYOffset);
}

void FVkDynamicUniformBufferApplication::OnKeyDown(int InKey, int InScanCode, int InMods)
{
	CameraController->OnKeyDown(InKey, InScanCode, InMods);
}

void FVkDynamicUniformBufferApplication::OnKeyUp(int InKey, int InScanCode, int InMods)
{
	CameraController->OnKeyUp(InKey, InScanCode, InMods);
}
