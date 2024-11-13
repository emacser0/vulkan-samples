#pragma once

#include "VulkanObject.h"

#include "Transform.h"

#include "glm/glm.hpp"

class FVulkanModel : public FVulkanObject
{
public:
	FVulkanModel(class FVulkanContext* InContext);

	class FVulkanMeshBase* GetMesh() const { return Mesh; }
	void SetMesh(class FVulkanMeshBase* InMesh) { Mesh = InMesh; }

	glm::mat4 GetModelMatrix() const { return Model; }
	void SetModelMatrix(const glm::mat4& InModel) { Model = InModel; }

protected:
	class FVulkanMeshBase* Mesh;

	glm::mat4 Model;
};
