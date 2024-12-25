#include "TextureCube.h"

#include "Engine.h"

#include "VulkanContext.h"
#include "VulkanTexture.h"

#include "stb_image.h"

UTextureCube::UTextureCube()
	: UTexture()
	, Width(0)
	, Height(0)
	, NumChannels(0)
	, Images({})
{

}

UTextureCube::~UTextureCube()
{

}

bool UTextureCube::Load(const std::array<std::string, 6>& InFilenames)
{
	for (int Idx = 0; Idx < InFilenames.size(); ++Idx)
	{
		const std::string& Filename = InFilenames[Idx];

		int OutWidth, OutHeight, OutNumChannels;

		Images[Idx] = stbi_load(Filename.c_str(), &OutWidth, &OutHeight, &OutNumChannels, STBI_rgb_alpha);
		if (Images[Idx] == nullptr)
		{
			Unload();
			return false;
		}

		if ((Width != 0 && Width != OutWidth) ||
			(Height != 0 && Height != OutHeight) ||
			(NumChannels != 0 && NumChannels != OutNumChannels))
		{
			Unload();
			return false;
		}

		Width = OutWidth;
		Height = OutHeight;
		NumChannels = OutNumChannels;

		stbi_set_flip_vertically_on_load(true);
	}

	CreateRenderTexture();

	return true;
}

void UTextureCube::Unload()
{	
	Width = 0;
	Height = 0;
	NumChannels = 0;

	for (int Idx = 0; Idx < Images.size(); ++Idx)
	{
		uint8_t* Pixels = Images[Idx];
		if (Pixels != nullptr)
		{
			stbi_image_free(Pixels);
		}

		Images[Idx] = nullptr;
	}

	DestroyRenderTexture();
}

void UTextureCube::CreateRenderTexture()
{
	FVulkanContext* RenderContext = GEngine->GetRenderContext();
	if (RenderContext != nullptr)
	{
		RenderTexture = RenderContext->CreateObject<FVulkanTexture>();
		RenderTexture->SetFormat(VK_FORMAT_R8G8B8A8_SRGB);
		RenderTexture->Load(this);
	}
}
