#pragma once

#include <memory>
#include <vector>

class FEngine
{
public:
	static void Init();
	static void Exit();

	FEngine();
	virtual ~FEngine();

	struct GLFWwindow* GetWindow() const { return Window; }
	std::shared_ptr<class FApplication> GetApplication() const { return Application; }
	class FVulkanContext* GetRenderContext() const { return RenderContext; }
	class FVulkanScene* GetScene() const { return Scene; }
	class FVulkanUIRenderer* GetUIRenderer() const { return UIRenderer; }

	void Run(std::shared_ptr<class FApplication> InApplication);
	void Tick(float InDeltaTime);

	const std::vector<std::shared_ptr<class FWidget>>& GetWidgets() const { return Widgets; }
	void AddWidget(const std::shared_ptr<class FWidget>& InWidget);
	void RemoveWidget(const std::shared_ptr<class FWidget>& InWidget);

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

	class FVulkanContext* RenderContext;
	class FVulkanScene* Scene;
	class FVulkanUIRenderer* UIRenderer;

	std::vector<std::shared_ptr<class FWidget>> Widgets;
};

extern FEngine* GEngine;
