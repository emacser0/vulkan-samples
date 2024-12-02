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
