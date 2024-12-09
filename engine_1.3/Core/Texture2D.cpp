#include "Texture2D.h"

#include "Engine.h"

#include "VulkanContext.h"
#include "VulkanTexture.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

UTexture2D::UTexture2D()
	: UTexture()
	, Width(0)
	, Height(0)
	, NumChannels(0)
	, bIsNormal(false)
	, Pixels(nullptr)
{

}

UTexture2D::~UTexture2D()
{
	if (Pixels != nullptr)
	{
		stbi_image_free(Pixels);
	}
}

bool UTexture2D::Load(const std::string& InFilename, bool InbIsNormal)
{
	int OutWidth, OutHeight, OutNumChannels;

	stbi_set_flip_vertically_on_load(true);

	Pixels = stbi_load(InFilename.c_str(), &OutWidth, &OutHeight, &OutNumChannels, STBI_rgb_alpha);
	if (Pixels == nullptr)
	{
		return false;
	}

	assert(OutWidth >= 0);
	assert(OutHeight >= 0);
	assert(OutNumChannels >= 0);

	Width = static_cast<uint32_t>(OutWidth);
	Height = static_cast<uint32_t>(OutHeight);
	NumChannels = static_cast<uint32_t>(OutNumChannels);
	bIsNormal = InbIsNormal;

	CreateRenderTexture();

	return true;
}

void UTexture2D::Unload()
{
	Width = 0;
	Height = 0;
	NumChannels = 0;
	bIsNormal = false;

	if (Pixels != nullptr)
	{
		stbi_image_free(Pixels);
		Pixels = nullptr;
	}

	DestroyRenderTexture();
}

void UTexture2D::CreateRenderTexture()
{
	FVulkanContext* RenderContext = GEngine->GetRenderContext();
	if (RenderContext != nullptr)
	{
		RenderTexture = RenderContext->CreateObject<FVulkanTexture>();
		RenderTexture->SetFormat(bIsNormal ? VK_FORMAT_R8G8B8A8_UNORM : VK_FORMAT_R8G8B8A8_SRGB);
		RenderTexture->Load(this);
	}
}
