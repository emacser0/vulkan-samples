#pragma once

#include "glm/glm.hpp"
#include "glfw/glfw3.h"

#include <memory>

class FCameraController
{
public:
	FCameraController(std::shared_ptr<class FCamera> InCamera);
	virtual ~FCameraController() = default;

	void Tick(float InDeltaTime);

	void OnMouseButtonDown(int InButton, int InMods);
	void OnMouseButtonUp(int InButton, int InMods);
	void OnMouseWheel(double InXOffset, double InYOffset);
	void OnKeyDown(int InKey, int InScanCode, int InMods);
	void OnKeyUp(int InKey, int InScanCode, int InMods);

private:
	std::shared_ptr<class FCamera> Camera;

	double PrevMouseX;
	double PrevMouseY;
	glm::vec3 RelativeMoveDelta;
	glm::vec3 AbsoluteMoveDelta;

	bool bRightButtonPressed;
};
