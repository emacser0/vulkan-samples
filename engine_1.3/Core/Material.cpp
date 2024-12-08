#include "Material.h"

#include "Engine.h"

#include "VulkanContext.h"
#include "VulkanMaterial.h"

UMaterial::UMaterial()
	: UAsset()
	, ShaderPath()
	, BaseColor()
	, Normal()
	, Ambient()
	, Diffuse()
	, Specular()
	, RenderMaterial()
{
}

UMaterial::~UMaterial()
{
	DestroyRenderMaterial();
}

FVulkanMaterial* UMaterial::GetRenderMaterial() const
{
	return RenderMaterial;
}

void UMaterial::CreateRenderMaterial()
{
	FVulkanContext* RenderContext = GEngine->GetRenderContext();
	if (RenderContext == nullptr)
	{
		return;
	}

	RenderMaterial = RenderContext->CreateObject<FVulkanMaterial>();

	if (RenderMaterial->LoadVS(ShaderPath.VS) == false)
	{
		DestroyRenderMaterial();
		return;
	}

	if (RenderMaterial->LoadFS(ShaderPath.FS) == false)
	{
		DestroyRenderMaterial();
		return;
	}

	RenderMaterial->SetBaseColor(BaseColor);
	RenderMaterial->SetNormal(Normal);
	RenderMaterial->SetAmbient(Ambient);
	RenderMaterial->SetDiffuse(Diffuse);
	RenderMaterial->SetSpecular(Specular);
}

void UMaterial::DestroyRenderMaterial()
{
	if (RenderMaterial == nullptr)
	{
		return;
	}

	FVulkanContext* RenderContext = GEngine->GetRenderContext();
	if (RenderContext == nullptr)
	{
		return;
	}

	RenderContext->DestroyObject(RenderMaterial);
	RenderMaterial = nullptr;
}
