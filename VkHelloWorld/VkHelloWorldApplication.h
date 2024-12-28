#pragma once

#include "Application.h"

#include <memory>

class FVkHelloWorldApplication : public FApplication
{
public:
	virtual void Run() override;
	virtual void Terminate() override;

	virtual void Tick(float InDeltaTime) override;
};
