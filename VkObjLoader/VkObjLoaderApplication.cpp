#include "VkObjLoaderApplication.h"
#include "Engine.h"
#include "Config.h"
#include "MeshUtils.h"
#include "SingleObjectRenderer.h"
#include "Camera.h"
#include "CameraController.h"

void FVkObjLoaderApplication::Run()
{
	GConfig->Set("CameraMoveSpeed", 1.0f);
	GConfig->Set("MouseSensitivity", 1.0f);

	Camera = std::make_shared<FCamera>();
	CameraController = std::make_shared<FCameraController>(Camera);
	Renderer = std::make_shared<FSingleObjectRenderer>();

	std::string MeshDirectory;
	GConfig->Get("MeshDirectory", MeshDirectory);

	LoadModel(MeshDirectory + "viking_room.obj", Renderer->GetVertices(), Renderer->GetIndices());

	std::string ImageDirectory;
	GConfig->Get("ImageDirectory", ImageDirectory);

	Renderer->GetTexture().Load(ImageDirectory + "viking_room.png");
	Renderer->Ready();
}

void FVkObjLoaderApplication::Terminate()
{
	Renderer->WaitIdle();
	Renderer = nullptr;
}

void FVkObjLoaderApplication::Tick(float InDeltaTime)
{
	CameraController->Tick(InDeltaTime);

	Renderer->SetViewMatrix(Camera->GetViewMatrix());
	Renderer->SetProjectionMatrix(Camera->GetProjectionMatrix());
	Renderer->Render(InDeltaTime);
}

void FVkObjLoaderApplication::OnMouseButtonDown(int InButton, int InMods)
{
	CameraController->OnMouseButtonDown(InButton, InMods);
}

void FVkObjLoaderApplication::OnMouseButtonUp(int InButton, int InMods)
{
	CameraController->OnMouseButtonUp(InButton, InMods);
}

void FVkObjLoaderApplication::OnMouseWheel(double InXOffset, double InYOffset)
{
	CameraController->OnMouseWheel(InXOffset, InYOffset);
}

void FVkObjLoaderApplication::OnKeyDown(int InKey, int InScanCode, int InMods)
{
	CameraController->OnKeyDown(InKey, InScanCode, InMods);
}

void FVkObjLoaderApplication::OnKeyUp(int InKey, int InScanCode, int InMods)
{
	CameraController->OnKeyUp(InKey, InScanCode, InMods);
}

