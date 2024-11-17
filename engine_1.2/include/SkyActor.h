#pragma once

#include "MeshActorBase.h"

#include "TextureSource.h"

#include <vector>

class ASkyActor : public AMeshActorBase
{
public:
	DECLARE_ACTOR_BODY(ASkyActor, AMeshActorBase);

	virtual class FVulkanModel* CreateRenderModel() override;

	void SetCubemap(const std::vector<FTextureSource*>& InCubemap);

private:
	std::vector<FTextureSource*> Cubemap;
};
