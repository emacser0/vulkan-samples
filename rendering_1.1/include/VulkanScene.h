#pragma once

#include "VulkanObject.h"
#include "VulkanLight.h"

#include <vector>

#include "glm/glm.hpp"

class FVulkanScene : public FVulkanObject
{
public:
	FVulkanScene(FVulkanContext* InContext);
	virtual ~FVulkanScene();

	const std::vector<class FVulkanModel*>& GetModels() const { return Models; }
	void AddModel(class FVulkanModel* InModel);
	void RemoveModel(class FVulkanModel* InModel);

	FVulkanLight GetLight() const { return Light; }
	void SetLight(const FVulkanLight& InLight);

private:
	std::vector<class FVulkanModel*> Models;
	FVulkanLight Light;
};
