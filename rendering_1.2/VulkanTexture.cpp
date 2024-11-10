#include "VulkanTexture.h"
#include "VulkanContext.h"
#include "VulkanHelpers.h"

#include "TextureSource.h"

#include <cassert>

FVulkanTexture::FVulkanTexture(FVulkanContext* InContext)
	: FVulkanObject(InContext)
	, Image(VK_NULL_HANDLE)
	, Memory(VK_NULL_HANDLE)
	, View(VK_NULL_HANDLE)
	, Width(0)
	, Height(0)
	, Channel(4)
	, Format(VK_FORMAT_R8G8B8A8_SRGB)
	, bLoaded(false)
{
}

FVulkanTexture::~FVulkanTexture()
{
	if (bLoaded)
	{
		VkDevice Device = Context->GetDevice();

		vkDestroyImage(Device, Image, nullptr);
		vkFreeMemory(Device, Memory, nullptr);
		vkDestroyImageView(Device, View, nullptr);
	}
}

void FVulkanTexture::LoadSource(FTextureSource* InSource)
{
	if (InSource == nullptr)
	{
		return;
	}

	if (InSource->GetWidth() <= 0 || InSource->GetHeight() <= 0)
	{
		return;
	}

	Width = static_cast<uint32_t>(InSource->GetWidth());
	Height = static_cast<uint32_t>(InSource->GetHeight());
	Depth = 1U;

	VkPhysicalDevice PhysicalDevice = Context->GetPhysicalDevice();
	VkDevice Device = Context->GetDevice();
	VkQueue GfxQueue = Context->GetGfxQueue();
	VkCommandPool CommandPool = Context->GetCommandPool();

	VkDeviceSize ImageSize = Width * Height * Channel;

	VkBuffer StagingBuffer = VK_NULL_HANDLE;
	VkDeviceMemory StagingBufferMemory = VK_NULL_HANDLE;
	Vk::CreateBuffer(
		PhysicalDevice,
		Device,
		ImageSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		StagingBuffer,
		StagingBufferMemory);

	void* Data = nullptr;

	VK_ASSERT(vkMapMemory(Device, StagingBufferMemory, 0, ImageSize, 0, &Data));
	memcpy(Data, InSource->GetPixels(), static_cast<size_t>(ImageSize));
	vkUnmapMemory(Device, StagingBufferMemory);

	Vk::CreateImage(
		PhysicalDevice,
		Device,
		Width,
		Height,
		Depth,
		Format,
		VK_IMAGE_TYPE_2D,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		Image,
		Memory);

	Vk::TransitionImageLayout(
		Device,
		CommandPool,
		GfxQueue,
		Image,
		Format,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	Vk::CopyBufferToImage(
		Device,
		CommandPool,
		GfxQueue,
		StagingBuffer,
		Image,
		Width,
		Height,
		Depth);
	Vk::TransitionImageLayout(
		Device,
		CommandPool,
		GfxQueue,
		Image,
		Format,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	vkDestroyBuffer(Device, StagingBuffer, nullptr);
	vkFreeMemory(Device, StagingBufferMemory, nullptr);

	View = Vk::CreateImageView(Device, Image, Format, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT);

	bLoaded = true;
}

void FVulkanTexture::LoadSource(const std::vector<FTextureSource*>& InSource)
{
	if (InSource.size() == 0)
	{
		return;
	}

	FTextureSource* FirstSource = InSource[0];
	if (FirstSource == nullptr)
	{
		return;
	}

	if (FirstSource->GetWidth() <= 0 || FirstSource->GetHeight() <= 0)
	{
		return;
	}

	VkPhysicalDevice PhysicalDevice = Context->GetPhysicalDevice();
	VkDevice Device = Context->GetDevice();
	VkQueue GfxQueue = Context->GetGfxQueue();
	VkCommandPool CommandPool = Context->GetCommandPool();

	VkDeviceSize SliceSize = Width * Height * Channel;
	VkDeviceSize ImageSize = SliceSize * Depth;

	Width = static_cast<uint32_t>(FirstSource->GetWidth());
	Height = static_cast<uint32_t>(FirstSource->GetHeight());
	Depth = static_cast<uint32_t>(InSource.size());

	VkBuffer StagingBuffer = VK_NULL_HANDLE;
	VkDeviceMemory StagingBufferMemory = VK_NULL_HANDLE;
	Vk::CreateBuffer(
		PhysicalDevice,
		Device,
		ImageSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		StagingBuffer,
		StagingBufferMemory);

	void* Data = nullptr;

	VK_ASSERT(vkMapMemory(Device, StagingBufferMemory, 0, ImageSize, 0, &Data));
	for (int Idx = 0; Idx < Depth; ++Idx)
	{
		FTextureSource* Source = InSource[Idx];
		if (Source == nullptr)
		{
			continue;
		}

		memcpy((uint8_t*)Data + SliceSize, Source->GetPixels(), static_cast<size_t>(ImageSize));
	}
	vkUnmapMemory(Device, StagingBufferMemory);

	Vk::CreateImage(
		PhysicalDevice,
		Device,
		Width,
		Height,
		Depth,
		Format,
		VK_IMAGE_TYPE_3D,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		Image,
		Memory);

	Vk::TransitionImageLayout(
		Device,
		CommandPool,
		GfxQueue,
		Image,
		Format,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	Vk::CopyBufferToImage(
		Device,
		CommandPool,
		GfxQueue,
		StagingBuffer,
		Image,
		Width,
		Height,
		Depth);
	Vk::TransitionImageLayout(
		Device,
		CommandPool,
		GfxQueue,
		Image,
		Format,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	vkDestroyBuffer(Device, StagingBuffer, nullptr);
	vkFreeMemory(Device, StagingBufferMemory, nullptr);

	View = Vk::CreateImageView(Device, Image, Format, VK_IMAGE_VIEW_TYPE_3D, VK_IMAGE_ASPECT_COLOR_BIT);

	bLoaded = true;
}

