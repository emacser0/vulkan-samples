#include "LightActor.h"
#include "Engine.h"

#include "Config.h"
#include "AssetManager.h"

#include "VulkanContext.h"
#include "VulkanModel.h"
#include "VulkanMesh.h"

ALightActor::ALightActor()
	: AMeshActorBase()
	, Ambient(0.0f, 0.0f, 0.0f, 1.0f)
	, Diffuse(0.0f, 0.0f, 0.0f, 1.0f)
	, Attenuation(1.0f, 0.0f, 0.0f, 1.0f)
	, Shininess(0.0f)
{
	std::string MeshDirectory;
	GConfig->Get("MeshDirectory", MeshDirectory);

	UMesh* SphereMeshAsset = FAssetManager::CreateAsset<UMesh>("Sphere");
	SphereMeshAsset->Load(MeshDirectory + "sphere.fbx");

	SetMeshAsset(SphereMeshAsset);
}

ALightActor::~ALightActor()
{
}

FVulkanModel* ALightActor::CreateRenderModel()
{
	if (RenderModel != nullptr)
	{
		return RenderModel;
	}

	FVulkanContext* RenderContext = GEngine->GetRenderContext();
	if (RenderContext == nullptr)
	{
		return nullptr;
	}

	RenderModel = RenderContext->CreateObject<FVulkanModel>();

	FVulkanMesh* NewMesh = RenderContext->CreateObject<FVulkanMesh>();
	NewMesh->Load(MeshAsset);

	RenderModel->SetMesh(NewMesh);

	return RenderModel;
}
