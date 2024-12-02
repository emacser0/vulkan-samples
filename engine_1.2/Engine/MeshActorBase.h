#pragma once

#include "Actor.h"
#include "Mesh.h"
#include "Material.h"

class AMeshActorBase : public AActor
{
public:
	DECLARE_ACTOR_BODY(AMeshActorBase, AActor);

	AMeshActorBase();

	UMesh* GetMesh() const { return Mesh; }
	void SetMesh(UMesh* InMesh) { Mesh = InMesh; }

	virtual class FVulkanModel* CreateRenderModel() { return nullptr; }
	void UpdateRenderModel();

	class FVulkanModel* GetRenderModel() const { return RenderModel; }

protected:
	UMesh* Mesh;

	class FVulkanModel* RenderModel;
};
