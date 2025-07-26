#pragma once

#include "glm/glm.hpp"

struct FVulkanPointLight
{
	alignas(16) glm::vec3 Position;
	alignas(16) glm::vec4 Ambient;
	alignas(16) glm::vec4 Diffuse;
	alignas(16) glm::vec4 Specular;
	alignas(16) glm::vec4 Attenuation;
	alignas(8) float Shininess;
	alignas(16) glm::mat4 LightSpaceMatrix;
};


struct FVulkanDirectionalLight
{
	alignas(16) glm::vec3 Direction;
	alignas(16) glm::vec4 Ambient;
	alignas(16) glm::vec4 Diffuse;
	alignas(16) glm::vec4 Specular;
	alignas(16) glm::vec4 Attenuation;
	alignas(8) float Shininess;
	alignas(16) glm::mat4 LightSpaceMatrix;
};
