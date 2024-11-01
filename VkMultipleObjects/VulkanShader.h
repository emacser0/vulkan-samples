#pragma once

#include "vulkan/vulkan.h"

#include <string>
#include <vector>

class FVulkanShader
{
public:
	FVulkanShader(class FVulkanContext* InContext);
	virtual ~FVulkanShader();

	bool LoadFile(const std::string& InFilename);
	bool LoadBytes(const std::vector<char>& InBytes);

	VkShaderModule GetModule() const { return ShaderModule;  }

private:
	class FVulkanContext* Context;

	VkShaderModule ShaderModule;
};
