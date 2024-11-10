#pragma once

#include "Actor.h"
#include "Transform.h"

#include "glm/glm.hpp"

class ACameraActor : public AActor
{
public:
	DECLARE_ACTOR_BODY(ACameraActor, AActor);

	ACameraActor();

	float GetFOV() const { return FOV; }
	void SetFOV(float InFOV) { FOV = InFOV; }

	float GetNear() const { return Near; }
	void SetNear(float InNear) { Near = InNear; }

	float GetFar() const { return Far; }
	void SetFar(float InFar) { Far = InFar; }

	glm::mat4 GetViewMatrix() const;

protected:
	float Near;
	float Far;
	float FOV;
};
