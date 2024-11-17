#include "SkyActor.h"
#include "Engine.h"

#include "VulkanModel.h"

#include "VulkanContext.h"
#include "VulkanModel.h"
#include "VulkanSkyMesh.h"
#include "VulkanTexture.h"

FVulkanModel* ASkyActor::CreateRenderModel()
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

	FVulkanSkyMesh* NewMesh = RenderContext->CreateObject<FVulkanSkyMesh>();
	NewMesh->Load(MeshAsset);

	RenderModel->SetMesh(NewMesh);

	if (Cubemap.size() > 0)
	{
		FVulkanTexture* NewTexture = RenderContext->CreateObject<FVulkanTexture>();
		NewTexture->LoadSource(Cubemap);
		NewMesh->SetCubemapTexture(NewTexture);
	}

	return RenderModel;
}

void ASkyActor::SetCubemap(const std::vector<FTextureSource*>& InSkyTexture)
{
	Cubemap = InSkyTexture;
}