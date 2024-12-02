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

	if (Mesh != nullptr)
	{
		FVulkanMesh* RenderMesh = Mesh->GetRenderMesh();
		if (RenderMesh != nullptr)
		{
			RenderModel->SetMesh(RenderMesh);
		}
	}

	return RenderModel;
}

void ASkyActor::SetCubemap(const std::vector<UTexture*>& InSkyTexture)
{
	Cubemap = InSkyTexture;
}