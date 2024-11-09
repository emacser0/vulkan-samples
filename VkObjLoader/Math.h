#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/hash.hpp"

#include <type_traits>

struct FVertex
{
	glm::vec3 Position;
	glm::vec3 Color;
	glm::vec2 TexCoords;

	bool operator==(const FVertex& RHS) const
	{
		return Position == RHS.Position && Color == RHS.Color && TexCoords == RHS.TexCoords;
	}
};

namespace std
{
	template<> struct hash<FVertex>
	{
		size_t operator()(const FVertex& InVertex) const
		{
            return ((hash<glm::vec3>()(InVertex.Position) ^ (hash<glm::vec3>()(InVertex.Color) << 1)) >> 1) ^ (hash<glm::vec2>()(InVertex.TexCoords) << 1);
		}
	};
}
