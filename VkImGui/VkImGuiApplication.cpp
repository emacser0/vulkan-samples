#include "VkImGuiApplication.h"
#include "Engine.h"
#include "Config.h"
#include "Utils.h"
#include "MeshUtils.h"
#include "Camera.h"
#include "CameraController.h"

#include "VulkanContext.h"
#include "VulkanMeshRenderer.h"
#include "VulkanUIRenderer.h"

#include <ctime>
#include <chrono>
#include <thread>
#include <filesystem>

void FVkImGuiApplication::Run()
{
	GConfig->Set("CameraMoveSpeed", 1.0f);
	GConfig->Set("MouseSensitivity", 1.0f);

	Camera = std::make_shared<FCamera>();
	CameraController = std::make_shared<FCameraController>(Camera);

	GLFWwindow* Window = GEngine->GetWindow();
	assert(Window != nullptr);

	RenderContext = new FVulkanContext(Window);
	UIRenderer = new FVulkanUIRenderer(RenderContext);
	MeshRenderer = new FVulkanMeshRenderer(RenderContext);

	UIRenderer->Ready();

	std::string MeshDirectory;
	GConfig->Get("MeshDirectory", MeshDirectory);

	LoadModel(MeshDirectory + "viking_room.obj", MeshRenderer->GetVertices(), MeshRenderer->GetIndices());

	std::string ImageDirectory;
	GConfig->Get("ImageDirectory", ImageDirectory);

	MeshRenderer->GetTexture().Source.Load(ImageDirectory + "viking_room.png");
	MeshRenderer->Ready();
}

void FVkImGuiApplication::Terminate()
{
	MeshRenderer->WaitIdle();

	delete UIRenderer;
	delete MeshRenderer;
	delete RenderContext;
}

void FVkImGuiApplication::Tick(float InDeltaTime)
{
	CameraController->Tick(InDeltaTime);

	MeshRenderer->SetViewMatrix(Camera->GetViewMatrix());
	MeshRenderer->SetProjectionMatrix(Camera->GetProjectionMatrix());

	RenderContext->BeginRender();
	MeshRenderer->Render();
	UIRenderer->Render();
	RenderContext->EndRender();
}

void FVkImGuiApplication::OnMouseButtonDown(int InButton, int InMods)
{
	CameraController->OnMouseButtonDown(InButton, InMods);
}

void FVkImGuiApplication::OnMouseButtonUp(int InButton, int InMods)
{
	CameraController->OnMouseButtonUp(InButton, InMods);
}

void FVkImGuiApplication::OnMouseWheel(double InXOffset, double InYOffset)
{
	CameraController->OnMouseWheel(InXOffset, InYOffset);
}

void FVkImGuiApplication::OnKeyDown(int InKey, int InScanCode, int InMods)
{
	CameraController->OnKeyDown(InKey, InScanCode, InMods);
}

void FVkImGuiApplication::OnKeyUp(int InKey, int InScanCode, int InMods)
{
	CameraController->OnKeyUp(InKey, InScanCode, InMods);
}

