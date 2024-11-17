#pragma once

#include "VulkanMeshBase.h"
#include "VulkanTexture.h"

#include "Vertex.h"

#include "vulkan/vulkan.h"

#include <cstdint>
#include <string>

class FVulkanMesh : public FVulkanMeshBase
{
public:
	FVulkanMesh(class FVulkanContext* InContext);

	FVulkanTexture* GetBaseColorTexture() const { return BaseColorTexture; }
	void SetBaseColorTexture(FVulkanTexture* InTexture) { BaseColorTexture = InTexture; }

	FVulkanTexture* GetNormalTexture() const { return NormalTexture; }
	void SetNormalTexture(FVulkanTexture* InTexture) { NormalTexture = InTexture; }

private:
	FVulkanTexture* BaseColorTexture;
	FVulkanTexture* NormalTexture;
};
