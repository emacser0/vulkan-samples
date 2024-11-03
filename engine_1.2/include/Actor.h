#pragma once

#include "Transform.h"

#include "glm/glm.hpp"

#include <string>

class AActor
{
public:
	static std::string StaticTypeId() { return "AActor"; }

	AActor(class FWorld* InWorld);
	AActor(class FWorld* InWorld, const std::string& InTypeId);

	virtual ~AActor();

	virtual void Initialize();
	virtual void Deinitialize();
	virtual void Tick(float DeltaTime);

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

#define DECLARE_ACTOR_BODY(ClassName) static std::string StaticTypeId() { return #ClassName; } \
	ClassName(class FWorld* InWorld) : AActor(InWorld, ClassName::StaticTypeId()) { } \



