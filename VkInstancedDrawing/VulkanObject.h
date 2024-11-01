#pragma once

class FVulkanObject
{
public:
	FVulkanObject(class FVulkanContext* InContext);
	virtual ~FVulkanObject();

protected:
	class FVulkanContext* Context;
};
