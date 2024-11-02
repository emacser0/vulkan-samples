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

	for (int Idx = 0; Idx < Indices.size(); Idx += 3)
	{
		const FVertex& V0 = Vertices[Indices[Idx]];
		const FVertex& V1 = Vertices[Indices[Idx + 1]];
		const FVertex& V2 = Vertices[Indices[Idx + 2]];

		glm::vec3 DeltaPos1 = V1.Position - V0.Position;
		glm::vec3 DeltaPos2 = V2.Position - V0.Position;

		glm::vec2 DeltaUV1 = V1.TexCoord - V0.TexCoord;
		glm::vec2 DeltaUV2 = V2.TexCoord - V0.TexCoord;

		float R = 1.0f / (DeltaUV1.x * DeltaUV2.y - DeltaUV1.y * DeltaUV2.x);
		glm::vec3 Tangent = (DeltaPos1 * DeltaUV2.y - DeltaPos2 * DeltaUV1.y) * R;

		for (int i = 0; i < 3; ++i)
		{
			Tangents.push_back(Tangent);
		}
	}

	return true;
}
