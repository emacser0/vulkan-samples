#include "VulkanTexture.h"
#include "VulkanContext.h"
#include "VulkanHelpers.h"

#include "Engine.h"
#include "TextureSource.h"

#include <cassert>

FVulkanTexture::FVulkanTexture(FVulkanContext* InContext)
	: FVulkanObject(InContext)
	, Image(VK_NULL_HANDLE)
	, Memory(VK_NULL_HANDLE)
	, View(VK_NULL_HANDLE)
	, Width(0)
	, Height(0)
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

void FVulkanTexture::LoadSource(const FTextureSource& InSource)
{
	if (InSource.GetWidth() <= 0)
	{
		return;
	}

	if (InSource.GetHeight() <= 0)
	{
		return;
	}

	if (InSource.GetPixels() == nullptr)
	{
		return;
	}

	Width = static_cast<uint32_t>(InSource.GetWidth());
	Height = static_cast<uint32_t>(InSource.GetHeight());

	FVulkanContext* Context = GEngine->GetRenderContext();

	VkPhysicalDevice PhysicalDevice = Context->GetPhysicalDevice();
	VkDevice Device = Context->GetDevice();
	VkQueue GfxQueue = Context->GetGfxQueue();
	VkCommandPool CommandPool = Context->GetCommandPool();

	VkDeviceSize ImageSize = Width * Height * 4U;

	VkBuffer StagingBuffer;
	VkDeviceMemory StagingBufferMemory;
	Vk::CreateBuffer(
		PhysicalDevice,
		Device,
		ImageSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		StagingBuffer,
		StagingBufferMemory);

	void* Data;

	VK_ASSERT(vkMapMemory(Device, StagingBufferMemory, 0, ImageSize, 0, &Data));
	memcpy(Data, InSource.GetPixels(), static_cast<size_t>(ImageSize));
	vkUnmapMemory(Device, StagingBufferMemory);

	Vk::CreateImage(
		PhysicalDevice,
		Device,
		Width,
		Height,
		Format,
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
		Height);
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

	View = Vk::CreateImageView(Device, Image, Format, VK_IMAGE_ASPECT_COLOR_BIT);

	bLoaded = true;
}
