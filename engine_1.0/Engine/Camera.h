#pragma once

#include "glm/glm.hpp"

#include "Transform.h"

class FCamera
{
public:
	FCamera();
	virtual ~FCamera();

	float GetFOV() const { return FOV; }
	FTransform GetTransform() const { return Transform; }

	void SetTransform(const FTransform& InTransform) { Transform = InTransform; }

	glm::vec3 GetLocation() const { return Transform.GetTranslation(); }
	glm::quat GetRotation() const { return Transform.GetRotation(); }
	glm::vec3 GetScale() const { return Transform.GetScale(); }

	void SetLocation(const glm::vec3& InLocation);
	void SetRotation(const glm::quat& InRotation);
	void SetScale(const glm::vec3& InScale);

	void AddOffset(const glm::vec3& InOffset);
	void AddRotation(const glm::quat& InRotation);
	void AddScale(const glm::vec3& InScale);

	glm::mat4 GetViewMatrix() const;

private:
	float FOV;
	FTransform Transform;
};
