#include "MeshActor.h"
#include "World.h"
#include "Engine.h"

#include "VulkanContext.h"
#include "VulkanModel.h"
#include "VulkanMesh.h"
#include "VulkanMaterial.h"
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

	if (MeshAsset == nullptr)
	{
		return nullptr;
	}

	if (Material == nullptr)
	{
		return nullptr;
	}

	FVulkanContext* RenderContext = GEngine->GetRenderContext();
	if (RenderContext == nullptr)
	{
		return nullptr;
	}

	RenderModel = RenderContext->CreateObject<FVulkanModel>();

	FVulkanMesh* NewMesh = RenderContext->CreateObject<FVulkanMesh>();
	NewMesh->Load(MeshAsset);

	FVulkanMaterial* NewMaterial = RenderContext->CreateObject<FVulkanMaterial>();
	FShaderPath ShaderPath = Material->GetShaderPath();

	if (NewMaterial->LoadVS(ShaderPath.VS) == false)
	{
		return nullptr;
	}

	if (NewMaterial->LoadFS(ShaderPath.FS) == false)
	{
		return nullptr;
	}

	NewMesh->SetMaterial(NewMaterial);

	if (BaseColor != nullptr)
	{
		FVulkanTexture* NewTexture = RenderContext->CreateObject<FVulkanTexture>();
		NewTexture->LoadSource(BaseColor);
		NewMesh->SetBaseColorTexture(NewTexture);
	}

	if (Normal != nullptr)
	{
		FVulkanTexture* NewTexture = RenderContext->CreateObject<FVulkanTexture>();
		NewTexture->SetFormat(VK_FORMAT_R8G8B8A8_UNORM);
		NewTexture->LoadSource(Normal);
		NewMesh->SetNormalTexture(NewTexture);
	}

	RenderModel->SetMesh(NewMesh);

	return RenderModel;
}
