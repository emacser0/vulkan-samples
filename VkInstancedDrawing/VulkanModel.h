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

	glm::mat4 GetCachedModelMatrix() const { return CachedModelMatrix; }

protected:
	void UpdateModelMatrix();

protected:
	class FVulkanMesh* Mesh;

	FTransform Transform;
	glm::mat4 CachedModelMatrix;
};
