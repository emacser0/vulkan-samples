#include "LightActor.h"

ALightActor::ALightActor()
	: AActor()
{
	Ambient = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	Diffuse = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	Specular = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	Attenuation = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	Shininess = 0.0f;
}
