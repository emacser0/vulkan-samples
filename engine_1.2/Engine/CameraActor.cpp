#include "CameraActor.h"
#include "Engine.h"
#include "Config.h"

#include "glfw/glfw3.h"

#include "glm/gtc/matrix_transform.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/quaternion.hpp"

ACameraActor::ACameraActor()
	: FOV(90.0f)
	, Near(0.01f)
	, Far(100.0f)
	, PrevMouseX(0.0f)
	, PrevMouseY(0.0f)
	, RelativeMoveDelta(0.0f)
{
}

void ACameraActor::Tick(float InDeltaTime)
{
	GLFWwindow* Window = GEngine->GetWindow();
	if (Window == nullptr)
	{
		return;
	}

	if (glfwGetMouseButton(Window, GLFW_MOUSE_BUTTON_RIGHT) != GLFW_PRESS)
	{
		return;
	}

	FWorld* World = GEngine->GetWorld();
	if (World == nullptr)
	{
		return;
	}

	glm::vec3 RotationDelta(0.0f);

	double MouseX, MouseY;
	glfwGetCursorPos(Window, &MouseX, &MouseY);

	double MouseDeltaX = MouseX - PrevMouseX;
	double MouseDeltaY = MouseY - PrevMouseY;

	float MouseSensitivity;
	GConfig->Get("MouseSensitivity", MouseSensitivity);

	float PitchAmount = MouseDeltaY * MouseSensitivity * InDeltaTime;
	float YawAmount = -MouseDeltaX * MouseSensitivity * InDeltaTime;

	if (abs(PitchAmount) > FLT_EPSILON || abs(YawAmount) > FLT_EPSILON)
	{
		glm::quat PitchRotation = glm::angleAxis(PitchAmount, glm::vec3(1.0f, 0.0f, 0.0f));
		glm::quat YawRotation = glm::angleAxis(YawAmount, glm::vec3(0.0f, 1.0f, 0.0f));

		SetRotation(YawRotation * GetRotation() * PitchRotation);
	}

	PrevMouseX = MouseX;
	PrevMouseY = MouseY;

	glm::mat4 RotationMatrix = glm::toMat4(GetRotation());

	float CameraMoveSpeed;
	GConfig->Get("CameraMoveSpeed", CameraMoveSpeed);

	glm::vec4 RelativeMoveVector = RotationMatrix * glm::vec4(glm::normalize(RelativeMoveDelta), 1.0f);
	glm::vec3 FinalRelativeMoveDelta(RelativeMoveVector);
	FinalRelativeMoveDelta = glm::normalize(FinalRelativeMoveDelta);

	if (glm::length(FinalRelativeMoveDelta) > FLT_EPSILON)
	{
		AddOffset(FinalRelativeMoveDelta * CameraMoveSpeed * InDeltaTime);
	}

	glm::vec3 FinalAbsoluteMoveDelta = glm::normalize(AbsoluteMoveDelta);

	if (glm::length(FinalAbsoluteMoveDelta) > FLT_EPSILON)
	{
		AddOffset(FinalAbsoluteMoveDelta * CameraMoveSpeed * InDeltaTime);
	}
}

void ACameraActor::OnMouseButtonDown(int InButton, int InMods)
{
	GLFWwindow* Window = GEngine->GetWindow();
	if (Window == nullptr)
	{
		return;
	}

	if (InButton == GLFW_MOUSE_BUTTON_RIGHT)
	{
		glfwSetInputMode(GEngine->GetWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		glfwGetCursorPos(GEngine->GetWindow(), &PrevMouseX, &PrevMouseY);
	}
}

void ACameraActor::OnMouseButtonUp(int InButton, int InMods)
{
	if (InButton == GLFW_MOUSE_BUTTON_RIGHT)
	{
		glfwSetInputMode(GEngine->GetWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}

}

void ACameraActor::OnMouseWheel(double InXOffset, double InYOffset)
{
	FWorld* World = GEngine->GetWorld();
	if (World == nullptr)
	{
		return;
	}

	if (abs(InYOffset) >= DBL_EPSILON)
	{
		glm::mat4 RotationMatrix = glm::toMat4(GetRotation());

		glm::vec4 MoveVector(0.0f, 0.0f, -InYOffset, 1.0f);

		float CameraMoveSpeed;
		GConfig->Get("CameraMoveSpeed", CameraMoveSpeed);

		AddOffset(glm::vec3(RotationMatrix * MoveVector) * CameraMoveSpeed * 0.1f);
	}
}

void ACameraActor::OnKeyDown(int InKey, int InScanCode, int InMods)
{
	if (InKey == GLFW_KEY_W)
	{
		RelativeMoveDelta.z = -1.0f;
	}
	else if (InKey == GLFW_KEY_S)
	{
		RelativeMoveDelta.z = 1.0f;
	}
	else if (InKey == GLFW_KEY_A)
	{
		RelativeMoveDelta.x = -1.0f;
	}
	else if (InKey == GLFW_KEY_D)
	{
		RelativeMoveDelta.x = 1.0f;
	}
	else if (InKey == GLFW_KEY_LEFT_CONTROL)
	{
		AbsoluteMoveDelta.y = 1.0f;
	}
	else if (InKey == GLFW_KEY_SPACE)
	{
		AbsoluteMoveDelta.y = -1.0f;
	}
}

void ACameraActor::OnKeyUp(int InKey, int InScanCode, int InMods)
{
	if (InKey == GLFW_KEY_W)
	{
		if (RelativeMoveDelta.z == -1.0f)
		{
			RelativeMoveDelta.z = 0.0f;
		}
	}
	else if (InKey == GLFW_KEY_S)
	{
		if (RelativeMoveDelta.z == 1.0f)
		{
			RelativeMoveDelta.z = 0.0f;
		}
	}
	else if (InKey == GLFW_KEY_A)
	{
		if (RelativeMoveDelta.x == -1.0f)
		{
			RelativeMoveDelta.x = 0.0f;
		}
	}
	else if (InKey == GLFW_KEY_D)
	{
		if (RelativeMoveDelta.x == 1.0f)
		{
			RelativeMoveDelta.x = 0.0f;
		}
	}
	else if (InKey == GLFW_KEY_LEFT_CONTROL)
	{
		if (AbsoluteMoveDelta.y == 1.0f)
		{
			AbsoluteMoveDelta.y = 0.0f;
		}
	}
	else if (InKey == GLFW_KEY_SPACE)
	{
		if (AbsoluteMoveDelta.y == -1.0f)
		{
			AbsoluteMoveDelta.y = 0.0f;
		}
	}
}

glm::mat4 ACameraActor::GetViewMatrix() const
{
	glm::mat4 RotationMatrix = glm::toMat4(Transform.GetRotation());
	glm::mat4 TranslationMatrix = glm::translate(glm::mat4(1.0f), Transform.GetTranslation());

	return glm::inverse(TranslationMatrix * RotationMatrix);
}
