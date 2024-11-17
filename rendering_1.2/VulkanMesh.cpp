#include "VulkanMesh.h"
#include "VulkanContext.h"
#include "VulkanHelpers.h"

#include "Mesh.h"

#include "glm/glm.hpp"

FVulkanMesh::FVulkanMesh(FVulkanContext* InContext)
	: FVulkanMeshBase(InContext)
	, BaseColorTexture(nullptr)
	, NormalTexture(nullptr)
{

}

