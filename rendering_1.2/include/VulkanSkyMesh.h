#pragma once

#include "VulkanMeshBase.h"
#include "VulkanTexture.h"

class FVulkanSkyMesh : public FVulkanMeshBase
{
public:
	FVulkanSkyMesh(class FVulkanContext* InContext);

	FVulkanTexture* GetCubemapTexture() const { return Cubemap; }
	void SetCubemapTexture(FVulkanTexture* InTexture) { Cubemap = InTexture; }

protected:
	FVulkanTexture* Cubemap;
};
