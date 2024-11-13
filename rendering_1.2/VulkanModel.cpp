#include "VulkanModel.h"
#include "VulkanContext.h"
#include "VulkanMeshBase.h"

#include "glm/gtc/matrix_transform.hpp"

FVulkanModel::FVulkanModel(class FVulkanContext* InContext)
	: FVulkanObject(InContext)
	, Mesh(nullptr)
	, Model(1.0f)
{
}
