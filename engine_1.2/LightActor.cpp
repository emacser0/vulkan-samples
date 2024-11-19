#include "LightActor.h"

ALightActor::ALightActor()
	: AActor()
	, Ambient(0.0f, 0.0f, 0.0f, 1.0f)
	, Diffuse(0.0f, 0.0f, 0.0f, 1.0f)
	, Attenuation(1.0f, 0.0f, 0.0f, 1.0f)
	, Shininess(0.0f)
{
}
