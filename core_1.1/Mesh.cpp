#include "Mesh.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include "glm/glm.hpp"

FMesh::FMesh()
	: FAsset()
{

}

FMesh::~FMesh()
{

}

bool FMesh::LoadObj(const std::string& InFilename)
{
	tinyobj::attrib_t Attributes;
	std::vector<tinyobj::shape_t> Shapes;
	std::vector<tinyobj::material_t> Materials;
	std::string Warn, Error;

	if (tinyobj::LoadObj(&Attributes, &Shapes, &Materials, &Warn, &Error, InFilename.c_str()) == false)
	{
		return false;
	}

	Vertices.clear();
	Indices.clear();

	std::unordered_map<FVertex, uint32_t> UniqueVertices;

	for (const tinyobj::shape_t& Shape : Shapes)
	{
		for (const tinyobj::index_t& Index : Shape.mesh.indices)
		{
			FVertex NewVertex{};
			NewVertex.Position = glm::vec3(
				Attributes.vertices[3 * Index.vertex_index + 0],
				Attributes.vertices[3 * Index.vertex_index + 1],
				Attributes.vertices[3 * Index.vertex_index + 2]);
			NewVertex.Normal = glm::vec3(
				Attributes.normals[3 * Index.normal_index + 0],
				Attributes.normals[3 * Index.normal_index + 1],
				Attributes.normals[3 * Index.normal_index + 2]);

			if (Index.texcoord_index != -1)
			{
				NewVertex.TexCoord = glm::vec2(
					Attributes.texcoords[2 * Index.texcoord_index + 0],
					1.0f - Attributes.texcoords[2 * Index.texcoord_index + 1]);
			}

			const auto Iter = UniqueVertices.find(NewVertex);
			if (Iter == UniqueVertices.end())
			{
				UniqueVertices[NewVertex] = static_cast<uint32_t>(UniqueVertices.size());
				Vertices.push_back(NewVertex);
			}

			Indices.push_back(UniqueVertices[NewVertex]);
		}
	}

	return true;
}
