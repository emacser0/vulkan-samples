#pragma once

#include "glm/glm.hpp"

struct FVulkanCamera
{
	glm::vec3 Position;
	glm::mat4 View;

	float FOV;
	float Near;
	float Far;
};
