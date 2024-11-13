#pragma once

#include "VulkanObject.h"
#include "VulkanLight.h"
#include "VulkanCamera.h"

#include <vector>

#include "glm/glm.hpp"

class FVulkanScene : public FVulkanObject
{
public:
	FVulkanScene(FVulkanContext* InContext);

	const std::vector<class FVulkanModel*>& GetModels() const { return Models; }
	void AddModel(class FVulkanModel* InModel);
	void RemoveModel(class FVulkanModel* InModel);
	void ClearModels();

	FVulkanLight GetLight() const { return Light; }
	void SetLight(const FVulkanLight& InLight) { Light = InLight; }

	FVulkanCamera GetCamera() const { return Camera; }
	void SetCamera(const FVulkanCamera& InCamera) { Camera = InCamera; }

private:
	std::vector<class FVulkanModel*> Models;
	FVulkanLight Light;
	FVulkanCamera Camera;
};
