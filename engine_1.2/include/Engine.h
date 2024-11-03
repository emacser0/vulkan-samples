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

private:
	struct GLFWwindow* Window;
	class FWorld* World;
	class FVulkanContext* RenderContext;
	class FVulkanUIRenderer* UIRenderer;
};

extern FEngine* GEngine;
