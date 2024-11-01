#include "VulkanModel.h"
#include "VulkanContext.h"
#include "VulkanMesh.h"

FVulkanModel::FVulkanModel(class FVulkanContext* InContext)
	: FVulkanObject(InContext)
	, Mesh(nullptr)
	, Transform()
{

}

FVulkanModel::~FVulkanModel()
{

}
