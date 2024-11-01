#pragma once

class FEngine
{
public:
	static void Init();
	static void Exit();

	FEngine();
	virtual ~FEngine();

	struct GLFWwindow* GetWindow() const;
	class FRenderer* GetRenderer() const;
	class FCamera* GetCamera() const;

private:
	void InitializeGLFW();
	void CreateGLFWWindow();

private:
	struct GLFWwindow* Window;
	class FRenderer* Renderer;
	class FCamera* Camera;
};

extern FEngine* GEngine;
