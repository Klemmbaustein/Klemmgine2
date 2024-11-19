#include "ModelConverter.h"
#include <Engine/File/ModelData.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <Engine/File/FileUtil.h>
#include <assimp/postprocess.h>
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
			.Position = Vector3(Pos.x, Pos.y, Pos.z) * Context.Options.ImportScale,
			.Normal = Normal ? Vector3(Normal->x, Normal->y, Normal->z) : 0,
			});
	}

	OutMesh.Indices.reserve(TargetMesh->mNumFaces * 3);
	for (uint32 i = 0; i < TargetMesh->mNumFaces; i++)
	{
		if (TargetMesh->mFaces[i].mNumIndices != 3)
			continue;

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
		| aiProcess_GenSmoothNormals
		| aiProcess_RemoveRedundantMaterials
		| aiProcess_PreTransformVertices
		| aiProcess_CalcTangentSpace
		| aiProcess_EmbedTextures;

	if (Options.Optimize)
	{
		Flags |= aiProcess_OptimizeMeshes
			| aiProcess_ImproveCacheLocality
			| aiProcess_FindDegenerates
			| aiProcess_JoinIdenticalVertices;
	}
	if (Options.GenerateUV)
	{
		Flags |= aiProcess_GenUVCoords;
	}

	if (Options.OnLoadStatusChanged)
	{
		Options.OnLoadStatusChanged("Reading file");
	}

	Assimp::Importer Import;
	ctx.ImportedScene = Import.ReadFile(ModelPath, 0);

	if (ctx.ImportedScene == nullptr)
		return "";

	ctx.Data = new ModelData();

	if (Options.OnLoadStatusChanged)
	{
		Options.OnLoadStatusChanged("Processing");
	}

	Import.ApplyPostProcessing(Flags);

	ProcessScene(ctx.ImportedScene->mRootNode, ctx);

	if (Options.OnLoadStatusChanged)
	{
		Options.OnLoadStatusChanged("Saving");
	}

	string OutFileName = file::FileNameWithoutExt(ModelPath) + ".kmdl";

	ctx.Data->ToFile(OutDir + OutFileName);

	delete ctx.Data;
	return OutDir + OutFileName;
}