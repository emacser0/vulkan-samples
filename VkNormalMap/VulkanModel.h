#pragma once

#include "VulkanObject.h"

#include "Transform.h"

#include "glm/glm.hpp"

class FVulkanModel : public FVulkanObject
{
public:
	FVulkanModel(class FVulkanContext* InContext);
	virtual ~FVulkanModel();

	class FVulkanMesh* GetMesh() const { return Mesh; }
	void SetMesh(class FVulkanMesh* InMesh) { Mesh = InMesh; }

	FTransform GetTransform() const { return Transform; }
	void SetTransform(const FTransform& InTransform);

	void SetLocation(const glm::vec3& InLocation);
	void SetRotation(const glm::quat& InRotation);
	void SetScale(const glm::vec3& InScale);

	void AddOffset(const glm::vec3& InOffset);
	void AddRotation(const glm::quat& InRotation);
	void AddScale(const glm::vec3& InScale);

	glm::mat4 GetCachedModelMatrix() const { return CachedModelMatrix; }

protected:
	void UpdateModelMatrix();

protected:
	class FVulkanMesh* Mesh;

	FTransform Transform;
	glm::mat4 CachedModelMatrix;
};
