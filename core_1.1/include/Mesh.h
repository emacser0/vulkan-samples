#pragma once

#include "Asset.h"
#include "Vertex.h"

#include <string>

class UMesh : public FAsset
{
public:
	UMesh();
	virtual ~UMesh();

	const std::vector<FVertex>& GetVertices() const { return Vertices; }
	const std::vector<uint32_t>& GetIndices() const { return Indices; }

	bool Load(const std::string& InFilename);

private:
	std::vector<FVertex> Vertices;
	std::vector<uint32_t> Indices;
};
