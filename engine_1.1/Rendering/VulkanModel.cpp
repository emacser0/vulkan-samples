#include "VulkanModel.h"
#include "VulkanContext.h"
#include "VulkanMesh.h"

#include "glm/gtc/matrix_transform.hpp"

FVulkanModel::FVulkanModel(class FVulkanContext* InContext)
	: FVulkanObject(InContext)
	, Mesh(nullptr)
	, Transform()
{
	UpdateModelMatrix();
}

FVulkanModel::~FVulkanModel()
{

}

void FVulkanModel::SetTransform(const FTransform& InTransform)
{
	Transform = InTransform;
	UpdateModelMatrix();
}

void FVulkanModel::UpdateModelMatrix()
{
	static const glm::mat4 IdentityMatrix(1.0f);
	CachedModelMatrix = glm::translate(IdentityMatrix, Transform.GetTranslation()) * glm::toMat4(Transform.GetRotation()) * glm::scale(IdentityMatrix, Transform.GetScale());
}
