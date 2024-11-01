#include "Camera.h"

FCamera GCamera;

void InitializeCamera()
{
	GCamera.FOV = 90.0f;
	GCamera.Position = glm::vec3(0.0f);
	GCamera.Rotation = glm::vec3(0.0f);
	GCamera.MoveSpeed = 1.0f;
	GCamera.MouseSensitivity = 1.0f;
}