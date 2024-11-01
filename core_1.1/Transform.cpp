#include "Transform.h"

FTransform::FTransform()
	: Translation(0.0f)
	, Rotation(1.0f, 0.0f, 0.0f, 0.0f)
	, Scale(1.0f)
{

}
FTransform::FTransform(const glm::vec3& InTranslation)
	: Translation(InTranslation)
	, Rotation(1.0f, 0.0f, 0.0f, 0.0f)
	, Scale(1.0f)
{

}

FTransform::FTransform(const glm::quat& InRotation)
	: Translation(0.0f)
	, Rotation(InRotation)
	, Scale(1.0f)
{

}

FTransform::FTransform(const glm::vec3& InTranslation, const glm::quat& InRotation)
	: Translation(InTranslation)
	, Rotation(InRotation)
	, Scale(1.0f)

{

}

FTransform::FTransform(const glm::vec3& InTranslation, const glm::quat& InRotation, const glm::vec3& InScale)
	: Translation(InTranslation)
	, Rotation(InRotation)
	, Scale(InScale)
{

}

glm::vec3 FTransform::GetRotator() const
{
	return glm::eulerAngles(Rotation);
}

glm::mat4 FTransform::ToMatrix() const
{
	return glm::mat4();
}
