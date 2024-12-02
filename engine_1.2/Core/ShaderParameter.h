#pragma once

#include "Texture.h"

#include "glm/glm.hpp"

enum class EShaderParameterType
{
	None,
	Float,
	Vector3,
	Vector4,
	Texture
};

struct FShaderParameter
{
	EShaderParameterType Type;

	float FloatParam;
	glm::vec3 Vec3Param;
	glm::vec4 Vec4Param;
	UTexture* TexParam;
};

