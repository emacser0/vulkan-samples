#pragma once

#include "Application.h"

#include <memory>

class FVkCubeApplication : public FApplication
{
public:
	virtual void Run() override;
	virtual void Terminate() override;

	virtual void Tick(float InDeltaTime) override;

	virtual void OnMouseButtonDown(int InButton, int InMods) override;
	virtual void OnMouseButtonUp(int InButton, int InMods) override;
	virtual void OnMouseWheel(double InXOffset, double InYOffset) override;
	virtual void OnKeyDown(int InKey, int InScanCode, int InMods) override;
	virtual void OnKeyUp(int InKey, int InScanCode, int InMods) override;

private:
	std::shared_ptr<class FCameraController> CameraController;
};
