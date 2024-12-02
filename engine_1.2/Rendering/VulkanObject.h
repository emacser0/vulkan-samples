#pragma once

class FVulkanObject
{
public:
	FVulkanObject(class FVulkanContext* InContext);
	virtual ~FVulkanObject();

	virtual void Destroy() { }

protected:
	class FVulkanContext* Context;
};
