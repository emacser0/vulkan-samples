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
	void SetBaseColor(const FShaderParameter& InBaseColor) { BaseColor = InBaseColor; }

	FShaderParameter GetNormal() const { return Normal; }
	void SetNormal(const FShaderParameter& InNormal) { Normal = InNormal; }

	FShaderParameter GetAmbient() const { return Ambient; }
	void SetAmbient(const FShaderParameter& InAmbient) { Ambient = InAmbient; }

	FShaderParameter GetDiffuse() const { return Diffuse; }
	void SetDiffuse(const FShaderParameter& InDiffuse) { Diffuse = InDiffuse; }

	FShaderParameter GetSpecular() const { return Specular; }
	void SetSpecular(const FShaderParameter& InSpecular) { Specular = InSpecular; }

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
