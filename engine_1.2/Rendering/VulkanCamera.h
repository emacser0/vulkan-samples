#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"

struct FVulkanCamera
{
	glm::vec3 Position;
	glm::quat Rotation;
	glm::mat4 View;

	float FOV;
	float Near;
	float Far;
};
