#pragma once

#include "VulkanObject.h"
#include "VulkanShader.h"

#include <string>

class FVulkanMaterial : public FVulkanObject
{
public:
	FVulkanMaterial(class FVulkanContext* InContext);

	bool LoadVS(const std::string& InFilename);
	bool LoadFS(const std::string& InFilename);

	void UnloadVS();
	void UnloadFS();
	void UnloadShaders();

	FVulkanShader* GetVS() const { return VS; }
	FVulkanShader* GetFS() const { return FS; }

	virtual void Destroy() override;

protected:
	FVulkanShader* VS;
	FVulkanShader* FS;
};
