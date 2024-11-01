#pragma once

class FCamera;

class FEngine
{
public:
	static void Init();
	static void Exit();

	FEngine();
	virtual ~FEngine();

	FCamera* GetCamera() { return Camera; }

private:
	FCamera* Camera;
};

extern FEngine* GEngine;
