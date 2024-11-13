#pragma once

#include "VulkanMeshBase.h"
#include "VulkanBuffer.h"

#include "Vertex.h"

#include "vulkan/vulkan.h"

#include <cstdint>
#include <string>

class FVulkanMesh : public FVulkanMeshBase
{
public:
	FVulkanMesh(class FVulkanContext* InContext);

	virtual void Destroy() override;

	virtual bool Load(class FMesh* InMesh) override;
	virtual void Unload() override;

	class FVulkanTexture* GetBaseColorTexture() const { return BaseColorTexture; }
	void SetBaseColorTexture(class FVulkanTexture* InTexture) { BaseColorTexture = InTexture; }

	class FVulkanTexture* GetNormalTexture() const { return NormalTexture; }
	void SetNormalTexture(class FVulkanTexture* InTexture) { NormalTexture = InTexture; }

private:
	class FVulkanTexture* BaseColorTexture;
	class FVulkanTexture* NormalTexture;

	bool bLoaded;
};
