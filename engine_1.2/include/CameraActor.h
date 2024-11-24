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
	
	virtual void Tick(float InDeltaTime) override;

	virtual void OnMouseButtonDown(int InButton, int InMods) override;
	virtual void OnMouseButtonUp(int InButton, int InMods) override;
	virtual void OnMouseWheel(double InXOffset, double InYOffset) override;
	virtual void OnKeyDown(int InKey, int InScanCode, int InMods) override;
	virtual void OnKeyUp(int InKey, int InScanCode, int InMods) override;

	glm::mat4 GetViewMatrix() const;

protected:
	float Near;
	float Far;
	float FOV;

	double PrevMouseX;
	double PrevMouseY;
	glm::vec3 MoveDelta;

};
