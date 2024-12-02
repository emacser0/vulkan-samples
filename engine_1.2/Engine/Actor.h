#pragma once

#include "Object.h"
#include "Transform.h"

#include "glm/glm.hpp"

#include <string>

#define DECLARE_ACTOR_BODY(ClassName, ParentClassName) \
	DECLARE_OBJECT_BODY(ClassName, ParentClassName); \
	static ClassName* StaticSpawnActor(class FWorld* InWorld) \
	{ \
		ClassName* NewActor = StaticCreateObject(); \
		NewActor->TypeId = ClassName::StaticTypeId(); \
		NewActor->SetWorld(InWorld); \
		return NewActor; \
	} \

class AActor : public UObject
{
public:
	DECLARE_ACTOR_BODY(AActor, UObject);

	AActor();
	virtual ~AActor() = default;

	virtual void Initialize();
	virtual void Deinitialize();
	virtual void Tick(float DeltaTime);

	virtual void OnMouseButtonDown(int InButton, int InMods) { }
	virtual void OnMouseButtonUp(int InButton, int InMods) { }
	virtual void OnMouseWheel(double InXOffset, double InYOffset) { }
	virtual void OnKeyDown(int InKey, int InScanCode, int InMods) { }
	virtual void OnKeyUp(int InKey, int InScanCode, int InMods) { }

	class FWorld* GetWorld() const;
	void SetWorld(class FWorld* InWorld);

	bool IsVisible() const { return bVisible; }
	void SetVisible(bool InbVisible) { bVisible = InbVisible; }

	FTransform GetTransform() const { return Transform; }
	void SetTransform(const FTransform& InTransform);

	glm::vec3 GetLocation() const { return Transform.GetTranslation(); }
	glm::quat GetRotation() const { return Transform.GetRotation(); }
	glm::vec3 GetScale() const { return Transform.GetScale(); }

	void SetLocation(const glm::vec3& InLocation);
	void SetRotation(const glm::quat& InRotation);
	void SetScale(const glm::vec3& InScale);

	void AddOffset(const glm::vec3& InOffset);
	void AddRotation(const glm::quat& InRotation);
	void AddScale(const glm::vec3& InScale);

	glm::mat4 GetCachedModelMatrix() const { return CachedModelMatrix; }

protected:
	void UpdateModelMatrix();

protected:
	class FWorld* World;

	FTransform Transform;
	glm::mat4 CachedModelMatrix;

	bool bVisible;
};

