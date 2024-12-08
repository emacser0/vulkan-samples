#include "Texture.h"

#include "Engine.h"

#include "VulkanContext.h"
#include "VulkanTexture.h"

#include <cassert>

UTexture::UTexture()
	: UAsset()
	, RenderTexture(nullptr)
{

}

UTexture::~UTexture()
{
	DestroyRenderTexture();
}

FVulkanTexture* UTexture::GetRenderTexture() const
{
	return RenderTexture;
}

void UTexture::DestroyRenderTexture()
{
	if (RenderTexture == nullptr)
	{
		return;
	}

	FVulkanContext* RenderContext = GEngine->GetRenderContext();
	if (RenderContext == nullptr)
	{
		return;
	}

	RenderContext->DestroyObject(RenderTexture);
	RenderTexture = nullptr;
}
