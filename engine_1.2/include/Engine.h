#pragma once

class FEngine
{
public:
	static void Init();
	static void Exit();

	FEngine();
	virtual ~FEngine();

	struct GLFWwindow* GetWindow() const;
	class FVulkanContext* GetRenderContext() const;
	class FVulkanScene* GetScene() const;
	class FVulkanUIRenderer* GetUIRenderer() const;
	class FCamera* GetCamera() const;

private:
	void InitializeGLFW();
	void CreateGLFWWindow();

private:
	struct GLFWwindow* Window;
	class FVulkanContext* RenderContext;
	class FVulkanScene* Scene;
	class FVulkanUIRenderer* UIRenderer;
	class FCamera* Camera;
};

extern FEngine* GEngine;
