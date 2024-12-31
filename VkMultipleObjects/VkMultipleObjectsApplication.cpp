#include "VkMultipleObjectsApplication.h"
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

#include <ctime>

FVkMultipleObjectsApplication::FVkMultipleObjectsApplication()
	: Camera(nullptr)
	, CameraController(nullptr)
	, MeshRenderer(nullptr)
	, Mesh(nullptr)
	, Texture(nullptr)
{
	srand(static_cast<unsigned int>(time(NULL)));

}

void FVkMultipleObjectsApplication::Run()
{
	GConfig->Set("CameraMoveSpeed", 1.0f);
	GConfig->Set("MouseSensitivity", 1.0f);

	Camera = std::make_shared<FCamera>();
	CameraController = std::make_shared<FCameraController>(Camera);

	std::string MeshDirectory;
	GConfig->Get("MeshDirectory", MeshDirectory);

	std::string ImageDirectory;
	GConfig->Get("ImageDirectory", ImageDirectory);

	FVulkanContext* RenderContext = GEngine->GetRenderContext();

	FTexture* TextureAsset = FAssetManager::CreateAsset<FTexture>();
	TextureAsset->Load(ImageDirectory + "wood.jpg");

	FVulkanTexture* Texture = new FVulkanTexture(RenderContext);
	Texture->LoadSource(*TextureAsset);

	FMesh* CubeMeshAsset = FAssetManager::CreateAsset<FMesh>();
	CubeMeshAsset->Load(MeshDirectory + "cube.obj");

	FVulkanMesh* Mesh = RenderContext->CreateObject<FVulkanMesh>();
	Mesh->Load(CubeMeshAsset);
	Mesh->SetTexture(Texture);

	for (int32_t Idx = 0; Idx < 50; ++Idx)
	{
		FVulkanModel* Model = new FVulkanModel(RenderContext);
		Model->SetMesh(Mesh);

		FTransform ModelTransform = Model->GetTransform();
		ModelTransform.SetTranslation(glm::vec3(RandRange(-25, 25), RandRange(-25, 25), RandRange(-25, 25)));
		ModelTransform.SetRotation(glm::vec3(glm::radians(90.0f), 0.0f, 0.0f));
		ModelTransform.SetScale(glm::vec3(1.0f, 1.0f, 1.0f));

		Model->SetTransform(ModelTransform);

		GEngine->GetScene()->AddModel(Model);
	}

	MeshRenderer = RenderContext->CreateObject<FVulkanMeshRenderer>();
}

void FVkMultipleObjectsApplication::Terminate()
{
	FVulkanContext* RenderContext = GEngine->GetRenderContext();
	RenderContext->WaitIdle();
	delete MeshRenderer;
}

void FVkMultipleObjectsApplication::Tick(float InDeltaTime)
{
	CameraController->Tick(InDeltaTime);

	MeshRenderer->SetViewMatrix(Camera->GetViewMatrix());
	MeshRenderer->SetProjectionMatrix(Camera->GetProjectionMatrix());

	FVulkanContext* RenderContext = GEngine->GetRenderContext();
	FVulkanUIRenderer* UIRenderer = GEngine->GetUIRenderer();

	RenderContext->BeginRender();
	MeshRenderer->Render();
	UIRenderer->Render(GEngine->GetWidgets());
	RenderContext->EndRender();
}

void FVkMultipleObjectsApplication::OnMouseButtonDown(int InButton, int InMods)
{
	CameraController->OnMouseButtonDown(InButton, InMods);
}

void FVkMultipleObjectsApplication::OnMouseButtonUp(int InButton, int InMods)
{
	CameraController->OnMouseButtonUp(InButton, InMods);
}

void FVkMultipleObjectsApplication::OnMouseWheel(double InXOffset, double InYOffset)
{
	CameraController->OnMouseWheel(InXOffset, InYOffset);
}

void FVkMultipleObjectsApplication::OnKeyDown(int InKey, int InScanCode, int InMods)
{
	CameraController->OnKeyDown(InKey, InScanCode, InMods);
}

void FVkMultipleObjectsApplication::OnKeyUp(int InKey, int InScanCode, int InMods)
{
	CameraController->OnKeyUp(InKey, InScanCode, InMods);
}
