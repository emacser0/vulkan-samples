#pragma once

#include "Actor.h"
#include "Mesh.h"

class AMeshActorBase : public AActor
{
public:
	DECLARE_ACTOR_BODY(AMeshActorBase, AActor);

	AMeshActorBase();

	UMesh* GetMeshAsset() const { return MeshAsset; }
	void SetMeshAsset(UMesh* InMeshAsset) { MeshAsset = InMeshAsset; }

	virtual class FVulkanModel* CreateRenderModel() { return nullptr; }
	void UpdateRenderModel();

	class FVulkanModel* GetRenderModel() const { return RenderModel; }

protected:
	UMesh* MeshAsset;

	class FVulkanModel* RenderModel;
};
