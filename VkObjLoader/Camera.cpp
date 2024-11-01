#include "Camera.h"

#include "glm/gtc/matrix_transform.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/quaternion.hpp"

FCamera GCamera;

FCamera::FCamera()
	: FOV(90.0f)
	, Transform()
{

}

FCamera::~FCamera()
{

}

glm::mat4 FCamera::GetViewMatrix() const
{
	glm::mat4 RotationMatrix = glm::toMat4(Transform.GetRotation());
	glm::mat4 TranslationMatrix = glm::translate(glm::mat4(1.0f), Transform.GetTranslation());

	return glm::inverse(TranslationMatrix * RotationMatrix);
}
