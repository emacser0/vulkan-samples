#include "VulkanModel.h"
#include "VulkanContext.h"
#include "VulkanMesh.h"

FVulkanModel::FVulkanModel(class FVulkanContext* InContext)
	: Context(InContext)
	, Mesh(nullptr)
	, Transform()
{

}

FVulkanModel::~FVulkanModel()
{

}
