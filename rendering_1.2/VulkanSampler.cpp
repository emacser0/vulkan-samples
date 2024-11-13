#include "VulkanSampler.h"
#include "VulkanContext.h"
#include "VulkanHelpers.h"

FVulkanSampler::FVulkanSampler(class FVulkanContext* InContext)
	: FVulkanObject(InContext)
{
	VkPhysicalDevice PhysicalDevice = Context->GetPhysicalDevice();
	VkDevice Device = Context->GetDevice();

	VkPhysicalDeviceProperties Properties{};
	vkGetPhysicalDeviceProperties(PhysicalDevice, &Properties);

	VkSamplerCreateInfo SamplerCI{};
	SamplerCI.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	SamplerCI.magFilter = VK_FILTER_LINEAR;
	SamplerCI.minFilter = VK_FILTER_LINEAR;
	SamplerCI.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	SamplerCI.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	SamplerCI.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	SamplerCI.anisotropyEnable = VK_TRUE;
	SamplerCI.maxAnisotropy = Properties.limits.maxSamplerAnisotropy;
	SamplerCI.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	SamplerCI.unnormalizedCoordinates = VK_FALSE;
	SamplerCI.compareEnable = VK_FALSE;
	SamplerCI.compareOp = VK_COMPARE_OP_ALWAYS;
	SamplerCI.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

	VK_ASSERT(vkCreateSampler(Device, &SamplerCI, nullptr, &Sampler));
}

void FVulkanSampler::Destroy()
{
	VkDevice Device = Context->GetDevice();
	
	if (Sampler != VK_NULL_HANDLE)
	{
		vkDestroySampler(Device, Sampler, nullptr);
	}
}
