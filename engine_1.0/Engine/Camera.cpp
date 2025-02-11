#include "Camera.h"
#include "Engine.h"

#include "glfw/glfw3.h"

#include "glm/gtc/matrix_transform.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/quaternion.hpp"

FCamera::FCamera()
	: FOV(90.0f)
	, Transform()
{

}

FCamera::~FCamera()
{

}


void FCamera::SetLocation(const glm::vec3& InLocation)
{
	Transform.SetTranslation(InLocation);
}

void FCamera::SetRotation(const glm::quat& InRotation)
{
	Transform.SetRotation(InRotation);
}

void FCamera::SetScale(const glm::vec3& InScale)
{
	Transform.SetScale(InScale);
}

void FCamera::AddOffset(const glm::vec3& InOffset)
{
	Transform.SetTranslation(Transform.GetTranslation() + InOffset);
}

void FCamera::AddRotation(const glm::quat& InRotation)
{
	Transform.SetRotation(InRotation * Transform.GetTranslation());
}

void FCamera::AddScale(const glm::vec3& InScale)
{
	Transform.SetScale(Transform.GetScale() + InScale);
}

glm::mat4 FCamera::GetViewMatrix() const
{
	glm::mat4 RotationMatrix = glm::toMat4(Transform.GetRotation());
	glm::mat4 TranslationMatrix = glm::translate(glm::mat4(1.0f), Transform.GetTranslation());

	return glm::inverse(TranslationMatrix * RotationMatrix);
}

glm::mat4 FCamera::GetProjectionMatrix() const
{
	float FOVRadians = glm::radians(FOV);

	GLFWwindow* Window = GEngine->GetWindow();
	assert(Window != nullptr);

	int Width, Height;
	glfwGetWindowSize(Window, &Width, &Height);

	float AspectRatio = Width / (float)Height;

	return glm::perspective(FOVRadians, AspectRatio, 0.01f, 100.0f);
}
