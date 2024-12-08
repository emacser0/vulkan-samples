#pragma once

#include "LightActor.h"

class APointLightActor : public ALightActor
{
public:
	DECLARE_ACTOR_BODY(APointLightActor, ALightActor);

	APointLightActor();
};
