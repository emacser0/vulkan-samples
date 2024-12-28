#pragma once

class FApplication
{
public:
	FApplication() = default;
	virtual ~FApplication() = default;

	virtual void Run() { }
	virtual void Terminate() { }

	virtual void Tick(float InDeltaTime) { }

	virtual void OnMouseButtonDown(int InButton, int InMods) { }
	virtual void OnMouseButtonUp(int InButton, int InMods) { }
	virtual void OnMouseWheel(double InXOffset, double InYOffset) { }
	virtual void OnKeyDown(int InKey, int InScanCode, int InMods) { }
	virtual void OnKeyUp(int InKey, int InScanCode, int InMods) { }
};
