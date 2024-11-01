#pragma once

#include "VulkanObject.h"

#include <vector>

#include "glm/glm.hpp"

struct FVulkanLight
{
	glm::vec3 Position;
	glm::vec4 Ambient;
	glm::vec4 Diffuse;
	glm::vec4 Specular;
	glm::vec4 Attenuation;
	float Shininess;
};

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
