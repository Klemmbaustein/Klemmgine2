#include "ModelConverter.h"
#include <Engine/File/ModelData.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>
using namespace engine;
using namespace engine::editor::modelConverter;

struct ConvertContext
{
	ModelData* Data = nullptr;
	const aiScene* ImportedScene = nullptr;
	ConvertOptions Options;
};

static void ProcessMesh(const aiMesh* TargetMesh, ConvertContext Context)
{
	Context.Data->Meshes.push_back(ModelData::Mesh());
	ModelData::Mesh& OutMesh = Context.Data->Meshes[Context.Data->Meshes.size() - 1];

	OutMesh.Vertices.reserve(TargetMesh->mNumVertices);
	for (uint32 i = 0; i < TargetMesh->mNumVertices; i++)
	{
		auto& Pos = TargetMesh->mVertices[i];

		aiVector3D* Normal = nullptr;
		if (TargetMesh->mNormals)
		{
			Normal = &TargetMesh->mNormals[i];
		}
		OutMesh.Vertices.push_back(Vertex{
			.Position = Vector3(Pos.x, Pos.y, Pos.z),
			.Normal = Normal ? Vector3(Normal->x, Normal->y, Normal->z) : 0,
			});
	}

	OutMesh.Indices.reserve(TargetMesh->mNumFaces * 3);
	for (uint32 i = 0; i < TargetMesh->mNumFaces; i++)
	{
		for (uint32 j = 0; j < TargetMesh->mFaces[i].mNumIndices; j++)
		{
			OutMesh.Indices.push_back(TargetMesh->mFaces[i].mIndices[j]);
		}
	}
}

static void ProcessScene(const aiNode* Node, ConvertContext Context)
{
	for (uint32 i = 0; i < Node->mNumMeshes; i++)
	{
		ProcessMesh(Context.ImportedScene->mMeshes[Node->mMeshes[i]], Context);
	}

	for (uint32 i = 0; i < Node->mNumChildren; i++)
	{
		ProcessScene(Node->mChildren[i], Context);
	}
}

string engine::editor::modelConverter::ConvertModel(string ModelPath, string OutDir, ConvertOptions Options)
{
	ConvertContext ctx;

	uint32 Flags = aiProcess_Triangulate
		| aiProcess_RemoveRedundantMaterials
		| aiProcess_CalcTangentSpace
		| aiProcess_GenNormals
		| aiProcess_EmbedTextures;

	if (Options.Optimize)
	{
		Flags |= aiProcess_OptimizeGraph
			| aiProcess_OptimizeMeshes
			| aiProcess_ImproveCacheLocality
			| aiProcess_FindDegenerates
			| aiProcess_JoinIdenticalVertices
			| aiProcess_RemoveRedundantMaterials;
	}
	if (Options.GenerateUV)
	{
		Flags |= aiProcess_GenUVCoords;
	}

	Assimp::Importer Import;
	ctx.ImportedScene = Import.ReadFile(ModelPath, Flags);

	if (ctx.ImportedScene == nullptr)
		return "";

	ctx.Data = new ModelData();

	ProcessScene(ctx.ImportedScene->mRootNode, ctx);

	ctx.Data->ToFile("Assets/importTest.kmdl");

	delete ctx.Data;
	return string();
}