#pragma once

#include "Actor.h"

#include "Mesh.h"
#include "Texture.h"

#include <vector>

class ASkyActor : public AActor
{
public:
	DECLARE_ACTOR_BODY(ASkyActor, AActor);

	UMesh* GetMesh() const { return Mesh; }
	void SetMesh(UMesh* InMesh) { Mesh = InMesh; }

	class FVulkanModel* CreateRenderModel();
	void UpdateRenderModel();

private:
	UMesh* Mesh;

	class FVulkanModel* RenderModel;
};
