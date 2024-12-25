#include "Utils.h"
#include "Math.h"
#include "Texture.h"

#include "glm/glm.hpp"

#include <unordered_map>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

bool LoadModel(const std::string& InFilename, std::vector<FVertex>& OutVertices, std::vector<uint16_t>& OutIndices)
{
	tinyobj::attrib_t Attributes;
	std::vector<tinyobj::shape_t> Shapes;
	std::vector<tinyobj::material_t> Materials;
	std::string Warn, Error;

	if (tinyobj::LoadObj(&Attributes, &Shapes, &Materials, &Warn, &Error, InFilename.c_str()) == false)
	{
		return false;
	}

	std::unordered_map<FVertex, uint16_t> UniqueVertices;

	for (const tinyobj::shape_t& Shape : Shapes)
	{
		for (const tinyobj::index_t& Index : Shape.mesh.indices)
		{
			FVertex NewVertex;
			NewVertex.Position = glm::vec3(
				Attributes.vertices[3 * Index.vertex_index + 0],
				Attributes.vertices[3 * Index.vertex_index + 1],
				Attributes.vertices[3 * Index.vertex_index + 2]);
			NewVertex.Color = glm::vec3(1.0f, 1.0f, 1.0f);
			NewVertex.TexCoords = glm::vec2(
				Attributes.texcoords[2 * Index.texcoord_index + 0],
				Attributes.texcoords[2 * Index.texcoord_index + 1]);

			const auto Iter = UniqueVertices.find(NewVertex);
			if (Iter == UniqueVertices.end())
			{
				UniqueVertices[NewVertex] = UniqueVertices.size();
				OutIndices.push_back(OutVertices.size());
				OutVertices.push_back(NewVertex);
			}
			else
			{
				OutIndices.push_back(Iter->second);
			}
		}
	}

	return true;
}

bool LoadTexture(const std::string& InFilename, FTexture& OutTexture)
{
	OutTexture.Pixels = stbi_load(InFilename.c_str(), &OutTexture.Width, &OutTexture.Height, &OutTexture.Channels, STBI_rgb_alpha);

	return OutTexture.Pixels != nullptr;
}
