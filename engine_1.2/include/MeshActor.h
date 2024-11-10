#pragma once

#include "MeshActorBase.h"
#include "TextureSource.h"

class AMeshActor : public AMeshActorBase
{
public:
	DECLARE_ACTOR_BODY(AMeshActor, AMeshActorBase);

	AMeshActor();

	class FTextureSource* GetBaseColorTexture() const { return BaseColor; }
	void SetBaseColorTexture(class FTextureSource* InTexture) { BaseColor = InTexture; }

	class FTextureSource* GetNormalTexture() const { return Normal; }
	void SetNormalTexture(class FTextureSource* InTexture) { Normal = InTexture; }

	virtual class FVulkanModel* CreateRenderModel() override;

protected:
	FTextureSource* BaseColor;
	FTextureSource* Normal;
};
