#include "VulkanTexture.h"
#include "VulkanContext.h"
#include "VulkanHelpers.h"
#include "VulkanImage.h"

#include "Texture2D.h"
#include "TextureCube.h"

#include <array>

FVulkanTexture::FVulkanTexture(FVulkanContext* InContext)
	: FVulkanObject(InContext)
	, Image(nullptr)
	, Width(0)
	, Height(0)
	, Channel(4U)
	, Format(VK_FORMAT_R8G8B8A8_SRGB)
{
}

void FVulkanTexture::Destroy()
{
	Unload();
}

void FVulkanTexture::Load(UTexture2D* InTexture)
{
	if (InTexture == nullptr)
	{
		return;
	}

	if (InTexture->GetWidth() <= 0 || InTexture->GetHeight() <= 0)
	{
		return;
	}

	Width = static_cast<uint32_t>(InTexture->GetWidth());
	Height = static_cast<uint32_t>(InTexture->GetHeight());
	Depth = 1U;
	Channel = 4U;

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
	memcpy(Data, InTexture->GetPixels(), static_cast<size_t>(ImageSize));
	vkUnmapMemory(Device, StagingBufferMemory);

	Image = Context->CreateObject<FVulkanImage>();
	Image->CreateImage(
		{ Width, Height, Depth },
		1,
		1,
		Format,
		VK_IMAGE_TYPE_2D,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	Image->CreateView(VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT);

	Vk::TransitionImageLayout(
		Device,
		CommandPool,
		GfxQueue,
		Image->GetImage(),
		1,
		1,
		Format,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	Vk::CopyBufferToImage(
		Device,
		CommandPool,
		GfxQueue,
		StagingBuffer,
		Image->GetImage(),
		0,
		1,
		{ Width, Height, Depth });
	Vk::TransitionImageLayout(
		Device,
		CommandPool,
		GfxQueue,
		Image->GetImage(),
		1,
		1,
		Format,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	vkDestroyBuffer(Device, StagingBuffer, nullptr);
	vkFreeMemory(Device, StagingBufferMemory, nullptr);
}

void FVulkanTexture::Load(UTextureCube* InTexture)
{
	if (InTexture == nullptr)
	{
		return;
	}

	Width = static_cast<uint32_t>(InTexture->GetWidth());
	Height = static_cast<uint32_t>(InTexture->GetHeight());
	Depth = 1U;
	Channel = 4U;

	uint32_t ArrayLayers = 6;

	VkPhysicalDevice PhysicalDevice = Context->GetPhysicalDevice();
	VkDevice Device = Context->GetDevice();
	VkQueue GfxQueue = Context->GetGfxQueue();
	VkCommandPool CommandPool = Context->GetCommandPool();

	VkDeviceSize SliceSize = Width * Height * Depth * Channel;
	VkDeviceSize ImageSize = SliceSize * ArrayLayers;

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

	const std::array<uint8_t*, 6>& Images = InTexture->GetImages();

	VK_ASSERT(vkMapMemory(Device, StagingBufferMemory, 0, ImageSize, 0, &Data));
	for (uint32_t Idx = 0; Idx < ArrayLayers; ++Idx)
	{
		uint8_t* Pixels = Images[Idx];
		if (Pixels == nullptr)
		{
			continue;
		}

		memcpy((uint8_t*)Data + SliceSize * Idx, Pixels, static_cast<size_t>(SliceSize));
	}
	vkUnmapMemory(Device, StagingBufferMemory);

	Image = Context->CreateObject<FVulkanImage>();
	Image->CreateImage(
		{ Width, Height, Depth },
		1,
		ArrayLayers,
		Format,
		VK_IMAGE_TYPE_2D,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT);
	Image->CreateView(VK_IMAGE_VIEW_TYPE_CUBE, VK_IMAGE_ASPECT_COLOR_BIT);

	Vk::TransitionImageLayout(
		Device,
		CommandPool,
		GfxQueue,
		Image->GetImage(),
		1,
		ArrayLayers,
		Format,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	Vk::CopyBufferToImage(
		Device,
		CommandPool,
		GfxQueue,
		StagingBuffer,
		Image->GetImage(),
		0,
		ArrayLayers,
		{ Width, Height, Depth });
	Vk::TransitionImageLayout(
		Device,
		CommandPool,
		GfxQueue,
		Image->GetImage(),
		1,
		ArrayLayers,
		Format,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	vkDestroyBuffer(Device, StagingBuffer, nullptr);
	vkFreeMemory(Device, StagingBufferMemory, nullptr);
}

void FVulkanTexture::Unload()
{
	if (Image != nullptr)
	{
		Context->DestroyObject(Image);
		Image = nullptr;
	}
}
