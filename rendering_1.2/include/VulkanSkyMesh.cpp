#include "VulkanSkyMesh.h"
#include "VulkanContext.h"
#include "VulkanTexture.h"

FVulkanSkyMesh::FVulkanSkyMesh(FVulkanContext* InContext)
	: FVulkanMeshBase(InContext)
	, Cubemap(nullptr)
{

}
