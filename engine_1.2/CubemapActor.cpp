#include "CubemapActor.h"
#include "Engine.h"

#include "VulkanModel.h"

#include "VulkanContext.h"
#include "VulkanModel.h"
#include "VulkanMesh.h"
#include "VulkanTexture.h"

FVulkanModel* ACubemapActor::CreateRenderModel()
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

	if (CubemapTextures.size() > 0)
	{
		FVulkanTexture* NewTexture = RenderContext->CreateObject<FVulkanTexture>();
		NewTexture->LoadSource(CubemapTextures);
		NewMesh->SetBaseColorTexture(NewTexture);
	}

	return RenderModel;
}

void ACubemapActor::SetCubemapTexture(const std::vector<FTextureSource*>& InCubemapTexture)
{
	CubemapTextures = InCubemapTexture;
}