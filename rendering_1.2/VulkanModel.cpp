#include "VulkanModel.h"
#include "VulkanContext.h"
#include "VulkanMesh.h"

#include "glm/gtc/matrix_transform.hpp"

FVulkanModel::FVulkanModel(class FVulkanContext* InContext)
	: FVulkanObject(InContext)
	, Mesh(nullptr)
{
}

FVulkanModel::~FVulkanModel()
{

}
