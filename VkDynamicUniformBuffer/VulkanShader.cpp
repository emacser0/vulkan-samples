#include "VulkanShader.h"
#include "VulkanContext.h"
#include "VulkanHelpers.h"

#include "Utils.h"

FVulkanShader::FVulkanShader(FVulkanContext* InContext)
	: FVulkanObject(InContext)
	, ShaderModule(VK_NULL_HANDLE)
{

}

FVulkanShader::~FVulkanShader()
{
	VkDevice Device = Context->GetDevice();

	if (ShaderModule != VK_NULL_HANDLE)
	{
		vkDestroyShaderModule(Device, ShaderModule, nullptr);
	}
}

bool FVulkanShader::LoadFile(const std::string& InFilename)
{
	std::vector<char> FileBytes;
	if (ReadFile(InFilename, FileBytes) == false)
	{
		return false;
	}

	return LoadBytes(FileBytes);
}

bool FVulkanShader::LoadBytes(const std::vector<char>& InBytes)
{
	VkDevice Device = Context->GetDevice();

	if (ShaderModule != VK_NULL_HANDLE)
	{
		vkDestroyShaderModule(Device, ShaderModule, nullptr);
	}

	VkShaderModuleCreateInfo ShaderModuleCI{};
	ShaderModuleCI.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	ShaderModuleCI.codeSize = InBytes.size();
	ShaderModuleCI.pCode = reinterpret_cast<const uint32_t*>(InBytes.data());

	return vkCreateShaderModule(Device, &ShaderModuleCI, nullptr, &ShaderModule) == VK_SUCCESS;
}
