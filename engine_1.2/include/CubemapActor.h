#pragma once

#include "MeshActorBase.h"

#include "TextureSource.h"

#include <vector>

class ACubemapActor : public AMeshActorBase
{
public:
	DECLARE_ACTOR_BODY(ACubemapActor, AMeshActorBase);

	ACubemapActor();

	virtual void Initialize() override;

	virtual class FVulkanModel* CreateRenderModel() override;

	void SetCubemapTexture(const std::vector<FTextureSource*>& InCubemapTexture);

private:
	std::vector<FTextureSource*> CubemapTextures;
};
