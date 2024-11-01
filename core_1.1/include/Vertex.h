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

struct FColorVertex
{
	glm::vec3 Position;
	glm::vec3 Color;
	glm::vec2 TexCoord;

	bool operator==(const FColorVertex& RHS) const;
};

struct FVertex
{
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoord;

	bool operator==(const FVertex& RHS) const;
};

struct FTangentVertex
{
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoord;
	glm::vec3 Tangent;

	bool operator==(const FTangentVertex& RHS) const;
};

namespace std
{
	template<> struct hash<FColorVertex>
	{
		size_t operator()(const FColorVertex& InVertex) const
		{
			size_t CombinedHash = hash<glm::vec3>()(InVertex.Position);
			CombineHash(CombinedHash, hash<glm::vec3>()(InVertex.Color));
			CombineHash(CombinedHash, hash<glm::vec2>()(InVertex.TexCoord));

			return CombinedHash;
		}
	};

	template<> struct hash<FVertex>
	{
		size_t operator()(const FVertex& InVertex) const
		{
			size_t CombinedHash = hash<glm::vec3>()(InVertex.Position);
			CombineHash(CombinedHash, hash<glm::vec3>()(InVertex.Normal));
			CombineHash(CombinedHash, hash<glm::vec2>()(InVertex.TexCoord));

			return CombinedHash;
		}
	};

	template<> struct hash<FTangentVertex>
	{
		size_t operator()(const FTangentVertex& InVertex) const
		{
			size_t CombinedHash = hash<glm::vec3>()(InVertex.Position);
			CombineHash(CombinedHash, hash<glm::vec3>()(InVertex.Normal));
			CombineHash(CombinedHash, hash<glm::vec2>()(InVertex.TexCoord));
			CombineHash(CombinedHash, hash<glm::vec2>()(InVertex.Tangent));

			return CombinedHash;
		}
	};
}
