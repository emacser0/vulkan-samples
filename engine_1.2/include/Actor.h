#pragma once

#include "Transform.h"

#include "glm/glm.hpp"

#include <string>

#define DECLARE_ACTOR_BODY_INTERNAL(ClassName) \
	static std::string StaticTypeId() { return #ClassName; } \
	static ClassName* StaticSpawnActor(class FWorld* InWorld) \
	{ \
		ClassName* NewActor = new ClassName(); \
		NewActor->TypeId = ClassName::StaticTypeId(); \
		NewActor->SetWorld(InWorld); \
		return NewActor; \
	} \

#define DECLARE_ACTOR_BODY(ClassName, ParentClassName) \
	DECLARE_ACTOR_BODY_INTERNAL(ClassName) \
	using Super = ParentClassName; \

class AActor
{
public:
	DECLARE_ACTOR_BODY_INTERNAL(AActor);

	AActor() { }
	virtual ~AActor() { }

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

	std::string GetTypeId() const { return TypeId; }

	glm::mat4 GetCachedModelMatrix() const { return CachedModelMatrix; }

protected:
	void UpdateModelMatrix();

protected:
	class FWorld* World;

	FTransform Transform;
	glm::mat4 CachedModelMatrix;
	
	std::string TypeId;
};

