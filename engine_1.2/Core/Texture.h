#pragma once

#include "Asset.h"

#include <cstdint>
#include <string>

class UTexture : public UAsset
{
public:
	DECLARE_OBJECT_BODY(UTexture, UAsset);

	UTexture();
	virtual ~UTexture();

	class FVulkanTexture* GetRenderTexture() const;
	virtual void CreateRenderTexture() { }
	void DestroyRenderTexture();

protected:
	class FVulkanTexture* RenderTexture;
};
