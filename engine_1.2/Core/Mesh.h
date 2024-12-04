#pragma once

#include "Asset.h"
#include "Vertex.h"
#include "Material.h"

#include <string>

class UMesh : public UAsset
{
public:
	DECLARE_OBJECT_BODY(UMesh, UAsset);

	UMesh();
	virtual ~UMesh();

	const std::vector<FVertex>& GetVertices() const { return Vertices; }
	const std::vector<uint32_t>& GetIndices() const { return Indices; }

	virtual bool Load(const std::string& InFilename);
	void Unload();

	UMaterial* GetMaterial() const { return Material; }
	void SetMaterial(UMaterial* InMaterial);

	class FVulkanMesh* GetRenderMesh() const;
	void CreateRenderMesh();
	void DestroyRenderMesh();

protected:
	std::vector<FVertex> Vertices;
	std::vector<uint32_t> Indices;

	UMaterial* Material;

	class FVulkanMesh* RenderMesh;
};
