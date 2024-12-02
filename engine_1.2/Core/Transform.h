#pragma once

#include "glm/glm.hpp"

#include "glm/gtc/matrix_transform.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/quaternion.hpp"

struct FTransform
{
public:
	FTransform();
	explicit FTransform(const glm::vec3& InTranslation);
	explicit FTransform(const glm::quat& InRotation);
	explicit FTransform(const glm::vec3& InTranslation, const glm::quat& InRotation);
	explicit FTransform(const glm::vec3& InTranslation, const glm::quat& InRotation, const glm::vec3& InScale);

	glm::vec3 GetTranslation() const { return Translation;  }
	glm::quat GetRotation() const { return Rotation; }
	glm::vec3 GetScale() const { return Scale;  }

	glm::vec3 GetRotator() const;

	void SetTranslation(const glm::vec3& InTranslation) { Translation = InTranslation;  }
	void SetRotation(const glm::quat& InRotation) { Rotation = InRotation;  }
	void SetRotation(const glm::vec3& InRotator) { Rotation = InRotator;  }
	void SetScale(const glm::vec3& InScale) { Scale = InScale;  }

	glm::mat4 ToMatrix() const;

protected:
	glm::vec3 Translation;
	glm::quat Rotation;
	glm::vec3 Scale;
};
