#pragma once

#include "VulkanContext.h"

class FVulkanObject
{
public:
	FVulkanObject(class FVulkanContext* InContext);
	virtual ~FVulkanObject();

protected:
	class FVulkanContext* Context;
};
