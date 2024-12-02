#pragma once

#include "MeshActorBase.h"

#include "Texture.h"

#include <vector>

class ASkyActor : public AMeshActorBase
{
public:
	DECLARE_ACTOR_BODY(ASkyActor, AMeshActorBase);

	virtual class FVulkanModel* CreateRenderModel() override;

	void SetCubemap(const std::vector<UTexture*>& InCubemap);

private:
	std::vector<UTexture*> Cubemap;
};
