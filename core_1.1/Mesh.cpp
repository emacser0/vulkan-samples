#include "Mesh.h"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "glm/glm.hpp"

FMesh::FMesh()
	: FAsset()
{

}

FMesh::~FMesh()
{

}

static aiMesh* FindFirstMesh(const aiScene* InScene, const aiNode* InNode)
{
	for (int Idx = 0; Idx < InNode->mNumMeshes; ++Idx)
	{
		aiMesh* Mesh = InScene->mMeshes[InNode->mMeshes[Idx]];
		if (Mesh != nullptr)
		{
			return Mesh;
		}
	}

	for (int Idx = 0; Idx < InNode->mNumChildren; ++Idx)
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

bool FMesh::Load(const std::string& InFilename)
{
	Assimp::Importer Importer;
	const aiScene* Scene = Importer.ReadFile(InFilename, aiProcess_Triangulate | aiProcess_FlipUVs);
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

	bool bHasTexCoords = Mesh->HasTextureCoords(0);
	bool bHasNormals = Mesh->HasNormals();

	for (int Idx = 0; Idx < Mesh->mNumVertices; ++Idx)
	{
		const aiVector3D& PositionData = Mesh->mVertices[Idx];

		FVertex NewVertex;
		NewVertex.Position = glm::vec3(PositionData.x, PositionData.y, PositionData.z);

		if (bHasNormals)
		{
			const aiVector3D& NormalData = Mesh->mNormals[Idx];
			NewVertex.Normal = glm::vec3(NormalData.x, NormalData.y, NormalData.z);
		}

		if (bHasTexCoords)
		{
			const aiVector3D& TexCoordsData = Mesh->mTextureCoords[0][Idx];
			NewVertex.TexCoords = glm::vec2(TexCoordsData.x, TexCoordsData.y);
		}

		Vertices.push_back(NewVertex);
	}

	for (int FaceIdx = 0; FaceIdx < Mesh->mNumFaces; ++FaceIdx)
	{
		const aiFace& Face = Mesh->mFaces[FaceIdx];
		for (int Idx = 0; Idx < Face.mNumIndices; ++Idx)
		{
			Indices.push_back(Face.mIndices[Idx]);
		}
	}

	return true;
}
