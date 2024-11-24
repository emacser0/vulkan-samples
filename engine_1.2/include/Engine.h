#pragma once

class FEngine
{
public:
	static void Init();
	static void Exit();

	FEngine();
	virtual ~FEngine();

	struct GLFWwindow* GetWindow() const;
	class FWorld* GetWorld() const;
	class FVulkanContext* GetRenderContext() const;
	class FVulkanUIRenderer* GetUIRenderer() const;

	void Tick(float DeltaTime);

private:
	void InitializeGLFW();
	void CreateGLFWWindow();
	void CompileShaders();

	static void OnMouseButtonEvent(GLFWwindow* InWindow, int InButton, int InAction, int InMods);
	static void OnMouseWheelEvent(GLFWwindow* InWindow, double InXOffset, double InYOffset);
	static void OnKeyEvent(GLFWwindow* InWindow, int InKey, int InScanCode, int InAction, int InMods);

private:
	struct GLFWwindow* Window;
	class FWorld* World;
	class FVulkanContext* RenderContext;
	class FVulkanUIRenderer* UIRenderer;
};

extern FEngine* GEngine;
