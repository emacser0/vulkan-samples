#include "MeshActor.h"
#include "World.h"
#include "Engine.h"

#include "VulkanContext.h"
#include "VulkanModel.h"
#include "VulkanMesh.h"
#include "VulkanTexture.h"

AMeshActor::AMeshActor()
	: AMeshActorBase()
	, BaseColor(nullptr)
	, Normal(nullptr)
{
}

FVulkanModel* AMeshActor::CreateRenderModel()
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

	if (BaseColor != nullptr)
	{
		FVulkanTexture* NewTexture = RenderContext->CreateObject<FVulkanTexture>();
		NewTexture->LoadSource(BaseColor);
		NewMesh->SetBaseColorTexture(NewTexture);
	}

	if (Normal != nullptr)
	{
		FVulkanTexture* NewTexture = RenderContext->CreateObject<FVulkanTexture>();
		NewTexture->LoadSource(Normal);
		NewMesh->SetNormalTexture(NewTexture);
	}

	RenderModel->SetMesh(NewMesh);

	return RenderModel;
}
