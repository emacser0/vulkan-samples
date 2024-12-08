#pragma once

#include "Asset.h"
#include "Texture.h"
#include "ShaderParameter.h"

#include "glm/glm.hpp"

#include <string>

struct FShaderPath
{
	std::string VS;
	std::string FS;
};

class UMaterial : public UAsset
{
public:
	DECLARE_OBJECT_BODY(UMaterial, UAsset);

	UMaterial();
	virtual ~UMaterial();

	FShaderPath GetShaderPath() const { return ShaderPath; }
	void SetShaderPath(const FShaderPath& InPath) { ShaderPath = InPath; }

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

	class FVulkanMaterial* GetRenderMaterial() const;
	void CreateRenderMaterial();
	void DestroyRenderMaterial();

private:
	FShaderPath ShaderPath;

	FShaderParameter BaseColor;
	FShaderParameter Normal;

	FShaderParameter Ambient;
	FShaderParameter Diffuse;
	FShaderParameter Specular;

	class FVulkanMaterial* RenderMaterial;
};
