#include "VulkanImage.h"
#include "VulkanContext.h"
#include "VulkanHelpers.h"

FVulkanImage::FVulkanImage(FVulkanContext* InContext)
	: FVulkanObject(InContext)
	, Extent({ 0, 0, 1 })
	, MipLevels(0)
	, ArrayLayers(0)
	, Format(VK_FORMAT_R8G8B8A8_UNORM)
	, ImageType(VK_IMAGE_TYPE_2D)
	, Tiling(VK_IMAGE_TILING_OPTIMAL)
	, Usage(0)
	, Properties(0)
	, Image(VK_NULL_HANDLE)
	, Memory(VK_NULL_HANDLE)
	, View(VK_NULL_HANDLE)
{
}

void FVulkanImage::Destroy()
{
	VkDevice Device = Context->GetDevice();

	if (Image != VK_NULL_HANDLE)
	{
		vkDestroyImage(Device, Image, nullptr);
	}

	if (Memory != VK_NULL_HANDLE)
	{
		vkFreeMemory(Device, Memory, nullptr);
	}

	if (View != VK_NULL_HANDLE)
	{
		vkDestroyImageView(Device, View, nullptr);
	}
}

void FVulkanImage::CreateImage(
	VkExtent3D InExtent,
	uint32_t InMipLevels,
	uint32_t InArrayLayers,
	VkFormat InFormat,
	VkImageType InImageType,
	VkImageTiling InTiling,
	VkImageUsageFlags InUsage,
	VkMemoryPropertyFlags InProperties,
	VkImageCreateFlags InFlags)
{
	Extent = InExtent;
	MipLevels = InMipLevels;
	ArrayLayers = InArrayLayers;
	Format = InFormat;
	ImageType = InImageType;
	Tiling = InTiling;
	Usage = InUsage;
	Properties = InProperties;

	VkImageCreateInfo ImageCI{};
	ImageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	ImageCI.extent = Extent;
	ImageCI.mipLevels = MipLevels;
	ImageCI.arrayLayers = ArrayLayers;
	ImageCI.format = Format;
	ImageCI.imageType = ImageType;
	ImageCI.tiling = Tiling;
	ImageCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	ImageCI.usage = Usage;
	ImageCI.samples = VK_SAMPLE_COUNT_1_BIT;
	ImageCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	ImageCI.flags = InFlags;

	VkDevice Device = Context->GetDevice();
	VkPhysicalDevice PhysicalDevice = Context->GetPhysicalDevice();

	VkResult Result = vkCreateImage(Device, &ImageCI, nullptr, &Image);

	VkMemoryRequirements MemoryReqs{};
	vkGetImageMemoryRequirements(Device, Image, &MemoryReqs);

	VkMemoryAllocateInfo MemoryAllocInfo{};
	MemoryAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	MemoryAllocInfo.allocationSize = MemoryReqs.size;
	MemoryAllocInfo.memoryTypeIndex = Vk::FindMemoryType(PhysicalDevice, MemoryReqs.memoryTypeBits, Properties);

	VK_ASSERT(vkAllocateMemory(Device, &MemoryAllocInfo, nullptr, &Memory));
	VK_ASSERT(vkBindImageMemory(Device, Image, Memory, 0));
}

void FVulkanImage::CreateView(
	VkImageViewType InImageViewType,
	VkImageAspectFlags InAspectFlags)
{
	if (Image == VK_NULL_HANDLE)
	{
		return;
	}

	VkDevice Device = Context->GetDevice();

	VkImageViewCreateInfo ImageViewCI{};
	ImageViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	ImageViewCI.image = Image;
	ImageViewCI.viewType = InImageViewType;
	ImageViewCI.format = Format;
	ImageViewCI.subresourceRange.aspectMask = InAspectFlags;
	ImageViewCI.subresourceRange.baseMipLevel = 0;
	ImageViewCI.subresourceRange.levelCount = MipLevels;
	ImageViewCI.subresourceRange.baseArrayLayer = 0;
	ImageViewCI.subresourceRange.layerCount = ArrayLayers;

	VK_ASSERT(vkCreateImageView(Device, &ImageViewCI, nullptr, &View));
}

void FVulkanImage::DestroyView()
{
	if (View == VK_NULL_HANDLE)
	{
		return;
	}

	VkDevice Device = Context->GetDevice();

	vkDestroyImageView(Device, View, nullptr);
}
