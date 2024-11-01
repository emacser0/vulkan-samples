#pragma once

#include "VulkanObject.h"

#include "Transform.h"

class FVulkanModel : public FVulkanObject
{
public:
	FVulkanModel(class FVulkanContext* InContext);
	virtual ~FVulkanModel();

	class FVulkanMesh* GetMesh() const { return Mesh; }
	void SetMesh(class FVulkanMesh* InMesh) { Mesh = InMesh; }

	FTransform GetTransform() const { return Transform; }
	void SetTransform(const FTransform& InTransform) { Transform = InTransform; }

private:
	class FVulkanMesh* Mesh;

	FTransform Transform;
};
