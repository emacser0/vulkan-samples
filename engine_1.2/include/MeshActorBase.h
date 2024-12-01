#pragma once

#include "Actor.h"
#include "Mesh.h"
#include "Material.h"

class AMeshActorBase : public AActor
{
public:
	DECLARE_ACTOR_BODY(AMeshActorBase, AActor);

	AMeshActorBase();

	UMesh* GetMeshAsset() const { return MeshAsset; }
	void SetMeshAsset(UMesh* InMeshAsset) { MeshAsset = InMeshAsset; }

	UMaterial* GetMaterial() const { return Material; }
	void SetMaterial(UMaterial* InMaterial) { Material = InMaterial; }

	virtual class FVulkanModel* CreateRenderModel() { return nullptr; }
	void UpdateRenderModel();

	class FVulkanModel* GetRenderModel() const { return RenderModel; }

protected:
	UMesh* MeshAsset;
	UMaterial* Material;

	class FVulkanModel* RenderModel;
};
