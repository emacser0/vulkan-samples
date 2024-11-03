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

	glm::mat4 GetModelMatrix() const { return Model; }
	void SetModelMatrix(const glm::mat4& InModel) { Model = InModel; }

protected:
	class FVulkanMesh* Mesh;

	glm::mat4 Model;
};
