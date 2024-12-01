#pragma once

#include "Asset.h"

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

private:
	FShaderPath ShaderPath;
};
