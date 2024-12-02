#include "SkyActor.h"
#include "Engine.h"

#include "VulkanModel.h"
#include "VulkanMesh.h"

#include "VulkanContext.h"
#include "VulkanModel.h"
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
	RenderModel->SetMesh(Mesh->GetRenderMesh());

	return RenderModel;
}

void ASkyActor::SetCubemap(const std::vector<UTexture*>& InSkyTexture)
{
	Cubemap = InSkyTexture;
}