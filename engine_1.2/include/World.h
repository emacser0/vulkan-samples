#pragma once

#include <vector>
#include <unordered_map>

class AActor;

class FWorld
{
public:
	FWorld();
	virtual ~FWorld();

	template <typename T = class AActor>
	T* SpawnActor()
	{
		T* NewActor = new T(this);
		Actors.push_back(NewActor);

		NewActor->Initialize();

		return NewActor;
	}

	void DestroyActor(AActor* InActor);

	void Tick(float DeltaTime);

	const std::vector<class AActor*>& GetActors();
	class ACameraActor* GetCamera() const;
	class ALightActor* GetLight() const;

	class FVulkanScene* GetRenderScene() const;

private:
	void GenerateRenderScene();
	void UpdateRenderScene();

private:
	std::vector<class AActor*> Actors;
	class ACameraActor* CameraActor;
	class ALightActor* LightActor;

	class FVulkanScene* RenderScene;
	std::unordered_map<class AActor*, class FVulkanModel*> RenderModelMap;
};
