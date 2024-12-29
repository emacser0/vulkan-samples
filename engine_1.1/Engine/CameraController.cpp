#include "CameraController.h"
#include "Config.h"
#include "Engine.h"
#include "Camera.h"

FCameraController::FCameraController(std::shared_ptr<FCamera> InCamera)
	: Camera(InCamera)
	, PrevMouseX(0.0f)
	, PrevMouseY(0.0f)
	, RelativeMoveDelta(0.0f)
	, AbsoluteMoveDelta(0.0f)
	, bRightButtonPressed(false)
{
}

void FCameraController::Tick(float InDeltaTime)
{
	GLFWwindow* Window = GEngine->GetWindow();
	assert(Window != nullptr);

	if (bRightButtonPressed == false)
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

		Camera->SetRotation(YawRotation * Camera->GetRotation() * PitchRotation);
	}

	PrevMouseX = MouseX;
	PrevMouseY = MouseY;

	glm::mat4 RotationMatrix = glm::toMat4(Camera->GetRotation());

	float CameraMoveSpeed;
	GConfig->Get("CameraMoveSpeed", CameraMoveSpeed);

	glm::vec4 RelativeMoveVector = RotationMatrix * glm::vec4(glm::normalize(RelativeMoveDelta), 1.0f);
	glm::vec3 FinalRelativeMoveDelta(RelativeMoveVector);
	FinalRelativeMoveDelta = glm::normalize(FinalRelativeMoveDelta);

	if (glm::length(FinalRelativeMoveDelta) > FLT_EPSILON)
	{
		Camera->AddOffset(FinalRelativeMoveDelta * CameraMoveSpeed * InDeltaTime);
	}

	glm::vec3 FinalAbsoluteMoveDelta = glm::normalize(AbsoluteMoveDelta);

	if (glm::length(FinalAbsoluteMoveDelta) > FLT_EPSILON)
	{
		Camera->AddOffset(FinalAbsoluteMoveDelta * CameraMoveSpeed * InDeltaTime);
	}
}

void FCameraController::OnMouseButtonDown(int InButton, int InMods)
{
	GLFWwindow* Window = GEngine->GetWindow();
	assert(Window != nullptr);

	if (InButton == GLFW_MOUSE_BUTTON_RIGHT)
	{
		glfwSetInputMode(Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		glfwGetCursorPos(Window, &PrevMouseX, &PrevMouseY);

		bRightButtonPressed = true;
	}
}

void FCameraController::OnMouseButtonUp(int InButton, int InMods)
{
	GLFWwindow* Window = GEngine->GetWindow();
	assert(Window != nullptr);

	if (InButton == GLFW_MOUSE_BUTTON_RIGHT)
	{
		glfwSetInputMode(Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

		bRightButtonPressed = false;
	}
}

void FCameraController::OnMouseWheel(double InXOffset, double InYOffset)
{
	if (abs(InYOffset) >= DBL_EPSILON)
	{
		glm::mat4 RotationMatrix = glm::toMat4(Camera->GetRotation());

		glm::vec4 MoveVector(0.0f, 0.0f, -InYOffset, 1.0f);

		float CameraMoveSpeed;
		GConfig->Get("CameraMoveSpeed", CameraMoveSpeed);

		Camera->AddOffset(glm::vec3(RotationMatrix * MoveVector) * CameraMoveSpeed * 0.1f);
	}
}

void FCameraController::OnKeyDown(int InKey, int InScanCode, int InMods)
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

void FCameraController::OnKeyUp(int InKey, int InScanCode, int InMods)
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

