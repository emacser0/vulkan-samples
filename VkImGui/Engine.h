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
	class FVulkanUIRenderer* GetUIRenderer() const;
	class FVulkanMeshRenderer* GetMeshRenderer() const;
	class FCamera* GetCamera() const;

private:
	void InitializeGLFW();
	void CreateGLFWWindow();

private:
	struct GLFWwindow* Window;
	class FVulkanContext* RenderContext;
	class FVulkanUIRenderer* UIRenderer;
	class FVulkanMeshRenderer* MeshRenderer;
	class FCamera* Camera;
};

extern FEngine* GEngine;
