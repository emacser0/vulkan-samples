#pragma once

#include "Asset.h"

#include "glm/glm.hpp"

#include <string>

class UMaterial : public UAsset
{
public:
	UMaterial();
	virtual ~UMaterial();

	void SetShaderPath(const std::string& InPath) { ShaderPath = InPath; }

private:
	std::string ShaderPath;
};
