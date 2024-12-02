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

	if (Mesh == nullptr)
	{
		return nullptr;
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
