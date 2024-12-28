#pragma once

#include <memory>

class FEngine
{
public:
	static void Init();
	static void Exit();

	FEngine();
	virtual ~FEngine();

	struct GLFWwindow* GetWindow() const { return Window; }
	std::shared_ptr<class FCamera> GetCamera() const { return Camera; }
	std::shared_ptr<class FApplication> GetApplication() const { return Application; }

	void Run(std::shared_ptr<class FApplication> InApplication);
	void Tick(float InDeltaTime);

private:
	void InitializeGLFW();
	void CreateGLFWWindow();
	void CompileShaders();

	static void OnMouseButtonEvent(GLFWwindow* InWindow, int InButton, int InAction, int InMods);
	static void OnMouseWheelEvent(GLFWwindow* InWindow, double InXOffset, double InYOffset);
	static void OnKeyEvent(GLFWwindow* InWindow, int InKey, int InScanCode, int InAction, int InMods);

private:
	struct GLFWwindow* Window;
	std::shared_ptr<class FApplication> Application;

	std::shared_ptr<class FCamera> Camera;
};

extern FEngine* GEngine;
