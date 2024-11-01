#pragma once

#include "VulkanObject.h"

#include <vector>

class FVulkanScene : public FVulkanObject
{
public:
	FVulkanScene(FVulkanContext* InContext);
	virtual ~FVulkanScene();

	const std::vector<class FVulkanModel*>& GetModels() const { return Models; }
	void AddModel(class FVulkanModel* InModel);
	void RemoveModel(class FVulkanModel* InModel);

private:
	std::vector<class FVulkanModel*> Models;
};
