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

	virtual bool Load(const std::string& InFilename);

protected:
	std::vector<FVertex> Vertices;
	std::vector<uint32_t> Indices;
};
