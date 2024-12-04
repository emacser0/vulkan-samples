#pragma once

#include "Actor.h"
#include "Texture.h"
#include "Mesh.h"
#include "Material.h"

class AMeshActor : public AActor
{
public:
	DECLARE_ACTOR_BODY(AMeshActor, AActor);

	AMeshActor();

	UMesh* GetMesh() const { return Mesh; }
	void SetMesh(UMesh* InMesh) { Mesh = InMesh; }

	class FVulkanModel* GetRenderModel() const;
	class FVulkanModel* CreateRenderModel();
	void UpdateRenderModel();

private:
	UMesh* Mesh;

	class FVulkanModel* RenderModel;
};
