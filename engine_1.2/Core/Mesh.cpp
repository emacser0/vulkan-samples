#include "Mesh.h"
#include "Engine.h"

#include "VulkanContext.h"
#include "VulkanMesh.h"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "glm/glm.hpp"

UMesh::UMesh()
	: UAsset()
	, Material(nullptr)
	, RenderMesh(nullptr)
{

}

UMesh::~UMesh()
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

bool UMesh::Load(const std::string& InFilename)
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
	bool bHasTangents = Mesh->HasTangentsAndBitangents();

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

		if (bHasTangents)
		{
			const aiVector3D& TangentData = Mesh->mTangents[Idx];
			NewVertex.Tangent = glm::vec3(TangentData.x, TangentData.y, TangentData.z);
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

	if (bHasTangents == false)
	{
		for (int Idx = 0; Idx < Indices.size(); Idx += 3)
		{
			FVertex& V0 = Vertices[Indices[Idx]];
			FVertex& V1 = Vertices[Indices[Idx + 1]];
			FVertex& V2 = Vertices[Indices[Idx + 2]];

			glm::vec3 DeltaPos1 = V1.Position - V0.Position;
			glm::vec3 DeltaPos2 = V2.Position - V0.Position;

			glm::vec2 DeltaUV1 = V1.TexCoords - V0.TexCoords;
			glm::vec2 DeltaUV2 = V2.TexCoords - V0.TexCoords;

			float R = 1.0f / (DeltaUV1.x * DeltaUV2.y - DeltaUV1.y * DeltaUV2.x);
			glm::vec3 Tangent;
			Tangent.x = (DeltaUV2.y * DeltaPos1.x - DeltaUV1.y * DeltaPos2.x) * R;
			Tangent.y = (DeltaUV2.y * DeltaPos1.y - DeltaUV1.y * DeltaPos2.y) * R;
			Tangent.z = (DeltaUV2.y * DeltaPos1.z - DeltaUV1.y * DeltaPos2.z) * R;
			Tangent = normalize(Tangent);

			V0.Tangent += Tangent;
			V1.Tangent += Tangent;
			V2.Tangent += Tangent;
		}
	}

	FVulkanContext* RenderContext = GEngine->GetRenderContext();
	if (RenderContext != nullptr)
	{
		RenderMesh = RenderContext->CreateObject<FVulkanMesh>();
		RenderMesh->Load(this);
	}

	return true;
}

void UMesh::Unload()
{
	FVulkanContext* RenderContext = GEngine->GetRenderContext();
	if (RenderContext != nullptr)
	{
		if (RenderMesh != nullptr)
		{
			RenderContext->DestroyObject(RenderMesh);
		}
	}
}

FVulkanMesh* UMesh::GetRenderMesh() const
{
	return RenderMesh;
}
