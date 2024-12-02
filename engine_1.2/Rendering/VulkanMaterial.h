#pragma once

#include "VulkanObject.h"
#include "VulkanShader.h"
#include "VulkanTexture.h"

#include "ShaderParameter.h"

#include "glm/glm.hpp"

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

	void Unload();

	FVulkanShader* GetVS() const { return VS; }
	FVulkanShader* GetFS() const { return FS; }

	FShaderParameter GetBaseColor() const { return BaseColor; }
	FShaderParameter GetNormal() const { return Normal; }

	void SetBaseColor(const FShaderParameter& InBaseColor) { BaseColor = InBaseColor; }
	void SetNormal(const FShaderParameter& InNormal) { Normal = InNormal; }

	virtual void Destroy() override;

protected:
	FVulkanShader* VS;
	FVulkanShader* FS;

	FShaderParameter BaseColor;
	FShaderParameter Normal;

	FShaderParameter Ambient;
	FShaderParameter Diffuse;
	FShaderParameter Specular;
};
