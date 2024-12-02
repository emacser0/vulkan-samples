#pragma once

#include "MeshActorBase.h"
#include "Texture.h"

class AMeshActor : public AMeshActorBase
{
public:
	DECLARE_ACTOR_BODY(AMeshActor, AMeshActorBase);

	AMeshActor();

	virtual class FVulkanModel* CreateRenderModel() override;
};
