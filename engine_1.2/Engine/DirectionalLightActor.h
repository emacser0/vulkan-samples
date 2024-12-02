#pragma once

#include "LightActor.h"

class ADirectionalLightActor : public ALightActor
{
public:
	DECLARE_ACTOR_BODY(ADirectionalLightActor, ALightActor);

	ADirectionalLightActor();

	glm::vec3 GetDirection() const { return Direction; }

	void SetDirection(const glm::vec3& InDirection) { Direction = InDirection; }

private:
	glm::vec3 Direction;
};
