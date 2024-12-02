#pragma once

#include "VulkanObject.h"

#include "vulkan/vulkan.h"

class FVulkanSampler : public FVulkanObject
{
public:
	FVulkanSampler(class FVulkanContext* InContext);

	virtual void Destroy() override;

	VkSampler GetSampler() const { return Sampler; }

private:
	VkSampler Sampler;
};
