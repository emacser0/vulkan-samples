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

	glm::mat4 GetViewMatrix() const;

private:
	float FOV;
	FTransform Transform;
};

extern FCamera GCamera;
