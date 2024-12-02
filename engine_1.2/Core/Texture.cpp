#include "Texture.h"

#include "Engine.h"

#include "VulkanContext.h"
#include "VulkanTexture.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <cassert>

UTexture::UTexture()
	: UAsset()
	, Width(0)
	, Height(0)
	, NumChannels(0)
	, Pixels(nullptr)
	, RenderTexture(nullptr)
{

}

UTexture::~UTexture()
{
	if (Pixels != nullptr)
	{
		stbi_image_free(Pixels);
	}
}

bool UTexture::Load(const std::string& InFilename, bool bIsNormal)
{
	int OutWidth, OutHeight, OutNumChannels;

	Pixels = stbi_load(InFilename.c_str(), &OutWidth, &OutHeight, &OutNumChannels, STBI_rgb_alpha);
	if (Pixels == nullptr)
	{
		return false;
	}

	stbi_set_flip_vertically_on_load(true);

	assert(OutWidth >= 0);
	assert(OutHeight >= 0);
	assert(OutNumChannels >= 0);

	Width = static_cast<uint32_t>(OutWidth);
	Height = static_cast<uint32_t>(OutHeight);
	NumChannels = static_cast<uint32_t>(OutNumChannels);

	FVulkanContext* RenderContext = GEngine->GetRenderContext();
	if (RenderContext != nullptr)
	{
		RenderTexture = RenderContext->CreateObject<FVulkanTexture>();
		RenderTexture->SetFormat(bIsNormal ? VK_FORMAT_R8G8B8A8_UNORM : VK_FORMAT_R8G8B8A8_SRGB);
		RenderTexture->LoadSource(this);
	}

	return true;
}

void UTexture::Unload()
{
	Width = 0;
	Height = 0;
	NumChannels = 0;

	if (Pixels != nullptr)
	{
		stbi_image_free(Pixels);
		Pixels = nullptr;
	}

	FVulkanContext* RenderContext = GEngine->GetRenderContext();
	if (RenderContext != nullptr)
	{
		if (RenderTexture != nullptr)
		{
			RenderContext->DestroyObject(RenderTexture);
			RenderTexture = nullptr;
		}
	}
}

FVulkanTexture* UTexture::GetRenderTexture() const
{
	return RenderTexture;
}
