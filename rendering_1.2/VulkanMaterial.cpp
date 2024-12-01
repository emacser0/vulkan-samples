#include "VulkanMaterial.h"
#include "VulkanContext.h"

FVulkanMaterial::FVulkanMaterial(FVulkanContext* InContext)
	: FVulkanObject(InContext)
	, VS(nullptr)
	, FS(nullptr)
{
}

bool FVulkanMaterial::LoadVS(const std::string& InFilename)
{
	if (VS != nullptr)
	{
		UnloadVS();
	}

	VS = Context->CreateObject<FVulkanShader>();
	return VS->LoadFile(InFilename);
}

bool FVulkanMaterial::LoadFS(const std::string& InFilename)
{
	if (FS != nullptr)
	{
		UnloadFS();
	}

	FS = Context->CreateObject<FVulkanShader>();
	return FS->LoadFile(InFilename);
}

void FVulkanMaterial::UnloadVS()
{
	if (VS == nullptr)
	{
		return;
	}

	Context->DestroyObject(VS);
	VS = nullptr;
}

void FVulkanMaterial::UnloadFS()
{
	if (FS == nullptr)
	{
		return;
	}

	Context->DestroyObject(FS);
	FS = nullptr;
}

void FVulkanMaterial::UnloadShaders()
{
	UnloadVS();
	UnloadFS();
}

void FVulkanMaterial::Destroy()
{
	UnloadShaders();
}
