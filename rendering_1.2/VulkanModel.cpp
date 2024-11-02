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

void FVulkanModel::SetLocation(const glm::vec3& InLocation)
{
	Transform.SetTranslation(InLocation);
	UpdateModelMatrix();
}

void FVulkanModel::SetRotation(const glm::quat& InRotation)
{
	Transform.SetRotation(InRotation);
	UpdateModelMatrix();
}

void FVulkanModel::SetScale(const glm::vec3& InScale)
{
	Transform.SetScale(InScale);
	UpdateModelMatrix();
}

void FVulkanModel::AddOffset(const glm::vec3& InOffset)
{
	Transform.SetTranslation(Transform.GetTranslation() + InOffset);
	UpdateModelMatrix();
}

void FVulkanModel::AddRotation(const glm::quat& InRotation)
{
	Transform.SetRotation(InRotation * Transform.GetTranslation());
	UpdateModelMatrix();
}

void FVulkanModel::AddScale(const glm::vec3& InScale)
{
	Transform.SetScale(Transform.GetScale() + InScale);
	UpdateModelMatrix();
}

void FVulkanModel::UpdateModelMatrix()
{
	static const glm::mat4 IdentityMatrix(1.0f);
	CachedModelMatrix = glm::translate(IdentityMatrix, Transform.GetTranslation()) * glm::toMat4(Transform.GetRotation()) * glm::scale(IdentityMatrix, Transform.GetScale());
}
