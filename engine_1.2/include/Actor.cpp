#include "Actor.h"
#include "World.h"

AActor::AActor(FWorld* InWorld)
	: World(InWorld)
	, TypeId(AActor::StaticTypeId())
{

}

AActor::AActor(class FWorld* InWorld, const std::string& InTypeId)
	: World(InWorld)
	, TypeId(InTypeId)
{

}

AActor::~AActor()
{

}

void AActor::Initialize()
{

}

void AActor::Deinitialize()
{

}

void AActor::Tick(float DeltatTime)
{

}

void AActor::SetTransform(const FTransform& InTransform)
{
	Transform = InTransform;
	UpdateModelMatrix();
}

void AActor::SetLocation(const glm::vec3& InLocation)
{
	Transform.SetTranslation(InLocation);
	UpdateModelMatrix();
}

void AActor::SetRotation(const glm::quat& InRotation)
{
	Transform.SetRotation(InRotation);
	UpdateModelMatrix();
}

void AActor::SetScale(const glm::vec3& InScale)
{
	Transform.SetScale(InScale);
	UpdateModelMatrix();
}

void AActor::AddOffset(const glm::vec3& InOffset)
{
	Transform.SetTranslation(Transform.GetTranslation() + InOffset);
	UpdateModelMatrix();
}

void AActor::AddRotation(const glm::quat& InRotation)
{
	Transform.SetRotation(InRotation * Transform.GetTranslation());
	UpdateModelMatrix();
}

void AActor::AddScale(const glm::vec3& InScale)
{
	Transform.SetScale(Transform.GetScale() + InScale);
	UpdateModelMatrix();
}

void AActor::UpdateModelMatrix()
{
	static const glm::mat4 IdentityMatrix(1.0f);
	CachedModelMatrix = glm::translate(IdentityMatrix, Transform.GetTranslation()) * glm::toMat4(Transform.GetRotation()) * glm::scale(IdentityMatrix, Transform.GetScale());
}
