#pragma once

#include "Actor.h"
#include "Mesh.h"
#include "TextureSource.h"

class AMeshActor : public AActor
{
public:
	DECLARE_ACTOR_BODY(AMeshActor);

	virtual void Initialize() override;

	FMesh* GetMeshAsset() const { return MeshAsset; }
	void SetMeshAsset(FMesh* InMeshAsset) { MeshAsset = InMeshAsset; }

	FTextureSource* GetBaseColorTexture() const { return BaseColor; }
	void SetBaseColorTexture(FTextureSource* InTexture) { BaseColor = InTexture; }

	FTextureSource* GetNormalTexture() const { return Normal; }
	void SetNormalTexture(FTextureSource* InTexture) { Normal = InTexture; }

protected:
	FMesh* MeshAsset;
	FTextureSource* BaseColor;
	FTextureSource* Normal;
};
