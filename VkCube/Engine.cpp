#include "Engine.h"
#include "Config.h"
#include "Camera.h"

FEngine* GEngine;

void FEngine::Init()
{
	GEngine = new FEngine();
	GConfig = new FConfig();
}

void FEngine::Exit()
{
	assert(GEngine != nullptr);
	delete GEngine;

	assert(GConfig != nullptr);
	delete GConfig;
}

FEngine::FEngine()
{
	Camera = new FCamera();
}

FEngine::~FEngine()
{
	delete Camera;
}
