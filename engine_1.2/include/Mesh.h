#pragma once

#include "Asset.h"
#include "Vertex.h"

#include <string>

class FMesh : public FAsset
{
public:
	FMesh();
	virtual ~FMesh();

	const std::vector<FVertex>& GetVertices() const { return Vertices; }
	const std::vector<uint32_t>& GetIndices() const { return Indices; }
	const std::vector<glm::vec3>& GetTangents() const { return Tangents; }

	bool LoadObj(const std::string& InFilename);

private:
	std::vector<FVertex> Vertices;
	std::vector<uint32_t> Indices;
	std::vector<glm::vec3> Tangents;
};
