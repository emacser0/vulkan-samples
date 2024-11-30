#pragma once

#include "MeshActorBase.h"
#include "Material.h"
#include "TextureSource.h"

class AMeshActor : public AMeshActorBase
{
public:
	DECLARE_ACTOR_BODY(AMeshActor, AMeshActorBase);

	AMeshActor();

	class UTextureSource* GetBaseColorTexture() const { return BaseColor; }
	void SetBaseColorTexture(class UTextureSource* InTexture) { BaseColor = InTexture; }

	class UTextureSource* GetNormalTexture() const { return Normal; }
	void SetNormalTexture(class UTextureSource* InTexture) { Normal = InTexture; }

	virtual class FVulkanModel* CreateRenderModel() override;

protected:
	UMaterial* Material;
	UTextureSource* BaseColor;
	UTextureSource* Normal;
};
