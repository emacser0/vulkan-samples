#include "World.h"
#include "Actor.h"
#include "Engine.h"
#include "CameraActor.h"
#include "LightActor.h"
#include "PointLightActor.h"
#include "DirectionalLightActor.h"
#include "SkyActor.h"
#include "MeshActor.h"

#include "VulkanContext.h"
#include "VulkanScene.h"
#include "VulkanModel.h"
#include "VulkanLight.h"
#include "VulkanCamera.h"
#include "VulkanMesh.h"
#include "VulkanTexture.h"

FWorld::FWorld()
	: CameraActor(nullptr)
	, RenderScene(nullptr)
{
	CameraActor = SpawnActor<ACameraActor>();
	SkyActor = SpawnActor<ASkyActor>();
}

FWorld::~FWorld()
{
	for (AActor* Actor : Actors)
	{
		if (Actor != nullptr)
		{
			Actor->Deinitialize();
			delete Actor;
		}
	}
}

void FWorld::DestroyActor(AActor* InActor)
{
	for (int Idx = 0; Idx < Actors.size(); ++Idx)
	{
		if (Actors[Idx] == InActor)
		{
			InActor->Deinitialize();
			Actors.erase(Actors.begin() + Idx);

			delete InActor;
			break;
		}
	}
}

void FWorld::Tick(float DeltaTime)
{
	if (RenderScene == nullptr)
	{
		GenerateRenderScene();
	}

	for (AActor* Actor : Actors)
	{
		if (Actor == nullptr)
		{
			continue;
		}

		Actor->Tick(DeltaTime);
	}

	UpdateRenderScene();
}

const std::vector<class AActor*>& FWorld::GetActors()
{
	return Actors;
}

ASkyActor* FWorld::GetSky() const
{
	return SkyActor;
}

ACameraActor* FWorld::GetCamera() const
{
	return CameraActor;
}

FVulkanScene* FWorld::GetRenderScene() const
{
	return RenderScene;
}

void FWorld::GenerateRenderScene()
{
	assert(GEngine != nullptr);

	FVulkanContext* RenderContext = GEngine->GetRenderContext();
	if (RenderContext == nullptr)
	{
		return;
	}

	RenderScene = RenderContext->CreateObject<FVulkanScene>();

	for (AActor* Actor : Actors)
	{
		if (Actor == nullptr)
		{
			continue;
		}

		if (Actor->GetTypeId() == AMeshActor::StaticTypeId())
		{
			if (AMeshActor* MeshActor = Cast<AMeshActor>(Actor))
			{
				FMesh* MeshAsset = MeshActor->GetMeshAsset();
				if (MeshAsset == nullptr)
				{
					continue;
				}

				FVulkanModel* Model = MeshActor->CreateRenderModel();
				if (Model != nullptr)
				{
					RenderScene->AddModel(Model);
				}
			}
		}
	}

	if (SkyActor != nullptr)
	{
		FVulkanModel* Model = SkyActor->CreateRenderModel();
		if (Model != nullptr)
		{
			RenderScene->SetSky(Model);
		}
	}
}

void FWorld::UpdateRenderScene()
{	
	assert(GEngine != nullptr);

	if (RenderScene == nullptr)
	{
		return;
	}

	std::vector<FVulkanPointLight> PointLights;
	std::vector<FVulkanDirectionalLight> DirectionalLights;

	if (CameraActor != nullptr)
	{
		FVulkanCamera Camera;
		Camera.Position = CameraActor->GetLocation();
		Camera.Rotation = CameraActor->GetRotation();
		Camera.FOV = CameraActor->GetFOV();
		Camera.Far = CameraActor->GetFar();
		Camera.Near = CameraActor->GetNear();
		Camera.View = CameraActor->GetViewMatrix();

		RenderScene->SetCamera(Camera);
	}

	for (AActor* Actor : Actors)
	{
		if (Actor == nullptr)
		{
			continue;
		}

		if (Actor->GetTypeId() == AMeshActor::StaticTypeId())
		{
			if (AMeshActor* MeshActor = Cast<AMeshActor>(Actor))
			{
				MeshActor->UpdateRenderModel();
			}
		}
		else if (Actor->GetTypeId() == APointLightActor::StaticTypeId())
		{
			if (APointLightActor* PointLight = Cast<APointLightActor>(Actor))
			{
				FVulkanPointLight Light;
				Light.Position = PointLight->GetLocation();
				Light.Ambient = PointLight->GetAmbient();
				Light.Diffuse = PointLight->GetDiffuse();
				Light.Specular = PointLight->GetSpecular();
				Light.Attenuation = PointLight->GetAttenuation();
				Light.Shininess = PointLight->GetShininess();

				PointLights.push_back(Light);
			}
		}
		else if (Actor->GetTypeId() == ADirectionalLightActor::StaticTypeId())
		{
			if (ADirectionalLightActor* DirectionalLight = Cast<ADirectionalLightActor>(Actor))
			{
				FVulkanDirectionalLight Light;
				Light.Direction = DirectionalLight->GetDirection();
				Light.Ambient = DirectionalLight->GetAmbient();
				Light.Diffuse = DirectionalLight->GetDiffuse();
				Light.Specular = DirectionalLight->GetSpecular();
				Light.Attenuation = DirectionalLight->GetAttenuation();
				Light.Shininess = DirectionalLight->GetShininess();

				DirectionalLights.push_back(Light);
			}
		}
	}

	RenderScene->SetPointLights(PointLights);
	RenderScene->SetDirectionalLights(DirectionalLights);

	if (SkyActor != nullptr)
	{
		SkyActor->UpdateRenderModel();
	}
}
