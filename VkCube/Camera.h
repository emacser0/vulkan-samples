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

	float GetPitch() const { return Pitch; }
	float GetYaw() const { return Yaw; }
	float GetRoll() const { return Roll; }

	void SetPitch(float InPitch);
	void SetYaw(float InYaw);
	void SetRoll(float InRoll);

private:
	float FOV;
	FTransform Transform;

	float Pitch;
	float Yaw;
	float Roll;
};

extern FCamera GCamera;
