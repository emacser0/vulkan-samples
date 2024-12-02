#pragma once

#include "MeshActorBase.h"

#include "glm/glm.hpp"

class ALightActor : public AActor
{
public:
	DECLARE_ACTOR_BODY(ALightActor, AActor);

	ALightActor();
	virtual ~ALightActor();

	glm::vec4 GetAmbient() const { return Ambient; }
	glm::vec4 GetDiffuse() const { return Diffuse; }
	glm::vec4 GetSpecular() const { return Specular; }
	glm::vec4 GetAttenuation() const { return Attenuation; }
	float GetShininess() const { return Shininess; }

	void SetAmbient(const glm::vec4& InAmbient) { Ambient = InAmbient; }
	void SetDiffuse(const glm::vec4& InDiffuse) { Diffuse = InDiffuse; }
	void SetSpecular(const glm::vec4& InSpecular) { Specular = InSpecular; }
	void SetAttenuation(const glm::vec4& InAttenuation) { Attenuation = InAttenuation; }
	void SetShininess(float InShininess) { Shininess = InShininess; }

protected:
	glm::vec4 Ambient;
	glm::vec4 Diffuse;
	glm::vec4 Specular;
	glm::vec4 Attenuation;
	float Shininess;
};
