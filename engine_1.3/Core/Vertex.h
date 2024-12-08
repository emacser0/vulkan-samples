#pragma once

#include "Utils.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/hash.hpp"

#include <type_traits>
#include <unordered_map>

struct FVertex
{
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoords;
	glm::vec3 Tangent;

	bool operator==(const FVertex& RHS) const;
};

namespace std
{
	template<> struct hash<FVertex>
	{
		size_t operator()(const FVertex& InVertex) const
		{
			size_t CombinedHash = hash<glm::vec3>()(InVertex.Position);
			CombineHash(CombinedHash, hash<glm::vec3>()(InVertex.Normal));
			CombineHash(CombinedHash, hash<glm::vec2>()(InVertex.Tangent));
			CombineHash(CombinedHash, hash<glm::vec2>()(InVertex.TexCoords));

			return CombinedHash;
		}
	};
}
