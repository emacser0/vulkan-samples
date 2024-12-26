#include "Utils.h"
#include "Math.h"
#include "Vertex.h"
#include "Texture.h"

#include "glm/glm.hpp"

#include <unordered_map>

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

static aiMesh* FindFirstMesh(const aiScene* InScene, const aiNode* InNode)
{
	for (uint32_t Idx = 0; Idx < InNode->mNumMeshes; ++Idx)
	{
		aiMesh* Mesh = InScene->mMeshes[InNode->mMeshes[Idx]];
		if (Mesh != nullptr)
		{
			return Mesh;
		}
	}

	for (uint32_t Idx = 0; Idx < InNode->mNumChildren; ++Idx)
	{
		aiNode* ChildNode = InNode->mChildren[Idx];
		if (ChildNode == nullptr)
		{
			continue;
		}

		aiMesh* FoundMesh = FindFirstMesh(InScene, ChildNode);
		if (FoundMesh != nullptr)
		{
			return FoundMesh;
		}
	}

	return nullptr;
}

bool LoadModel(const std::string& InFilename, std::vector<FVertex>& OutVertices, std::vector<uint32_t>& OutIndices)
{
	Assimp::Importer Importer;
	const aiScene* Scene = Importer.ReadFile(InFilename, aiProcess_Triangulate);
	if (Scene == nullptr)
	{
		return false;
	}

	aiNode* RootNode = Scene->mRootNode;
	if (RootNode == nullptr)
	{
		return false;
	}

	aiMesh* Mesh = FindFirstMesh(Scene, RootNode);
	if (Mesh == nullptr)
	{
		return false;
	}

	bool bHasColor = Mesh->HasVertexColors(0);
	bool bHasTexCoords = Mesh->HasTextureCoords(0);

	for (uint32_t Idx = 0; Idx < Mesh->mNumVertices; ++Idx)
	{
		const aiVector3D& PositionData = Mesh->mVertices[Idx];
		const aiColor4D& ColorData = Mesh->mColors[0][Idx];

		FVertex NewVertex{};
		NewVertex.Position = glm::vec3(PositionData.x, PositionData.y, PositionData.z);

		if (bHasColor)
		{
			NewVertex.Color = glm::vec4(ColorData.r, ColorData.g, ColorData.b, ColorData.a);
		}

		if (bHasTexCoords)
		{
			const aiVector3D& TexCoordsData = Mesh->mTextureCoords[0][Idx];
			NewVertex.TexCoords = glm::vec2(TexCoordsData.x, TexCoordsData.y);
		}

		OutVertices.push_back(NewVertex);
	}

	for (int FaceIdx = 0; FaceIdx < Mesh->mNumFaces; ++FaceIdx)
	{
		const aiFace& Face = Mesh->mFaces[FaceIdx];
		for (uint32_t Idx = 0; Idx < Face.mNumIndices; ++Idx)
		{
			OutIndices.push_back(Face.mIndices[Idx]);
		}
	}

	return true;
}
