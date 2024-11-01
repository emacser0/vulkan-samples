#pragma once

#include "glm/glm.hpp"

class FCamera
{
public:
	float FOV;
	glm::vec3 Position;
	glm::vec3 Rotation;
	float MoveSpeed;
	float MouseSensitivity;
};

extern FCamera GCamera;

void InitializeCamera();
