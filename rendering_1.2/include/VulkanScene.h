#pragma once

#include "VulkanObject.h"
#include "VulkanLight.h"
#include "VulkanCamera.h"
#include "VulkanModel.h"

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

	FVulkanModel* GetSky() const { return Sky; }
	void SetSky(FVulkanModel* InMesh) { Sky = InMesh; }

	const std::vector<FVulkanPointLight>& GetPointLights() const { return PointLights; }
	void SetPointLights(const std::vector<FVulkanPointLight>& InPointLights) { PointLights = InPointLights; }

	const std::vector<FVulkanDirectionalLight>& GetDirectionalLights() const { return DirectionalLights; }
	void SetDirectionalLights(const std::vector<FVulkanDirectionalLight>& InDirectionalLights) { DirectionalLights = InDirectionalLights; }

	FVulkanCamera GetCamera() const { return Camera; }
	void SetCamera(const FVulkanCamera& InCamera) { Camera = InCamera; }

private:
	std::vector<class FVulkanModel*> Models;
	FVulkanModel* Sky;

	FVulkanCamera Camera;

	std::vector<FVulkanPointLight> PointLights;
	std::vector<FVulkanDirectionalLight> DirectionalLights;
};
