#ifdef EDITOR
#include "ModelConverter.h"
#include <assimp/cimport.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <Core/File/FileUtil.h>
#include <Core/Log.h>
#include <Engine/File/ModelData.h>
#include <fstream>
using namespace engine;
using namespace engine::editor::modelConverter;

struct ConvertContext
{
	ModelData* Data = nullptr;
	string SceneName;
	const aiScene* ImportedScene = nullptr;
	ConvertOptions Options;
};

static void ProcessMesh(const aiMesh* TargetMesh, ConvertContext Context, string OutDir)
{
	ModelData::Mesh& OutMesh = Context.Data->Meshes.emplace_back(true);

	OutMesh.Vertices.reserve(TargetMesh->mNumVertices);
	for (uint32 i = 0; i < TargetMesh->mNumVertices; i++)
	{
		auto& Pos = TargetMesh->mVertices[i];

		aiVector3D* Normal = nullptr;
		aiVector3D* UV = nullptr;
		if (TargetMesh->mNormals)
		{
			Normal = &TargetMesh->mNormals[i];
		}
		if (TargetMesh->mTextureCoords[0])
		{
			UV = &TargetMesh->mTextureCoords[0][i];
		}
		OutMesh.Vertices.push_back(graphics::Vertex{
			.Position = Vector3(Pos.x, Pos.y, Pos.z) * Context.Options.ImportScale,
			.UV = UV ? Vector2(UV->x, UV->y) : 0,
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

	string MaterialPath = str::Format("%s_%i", Context.SceneName.c_str(), int(TargetMesh->mMaterialIndex));
	OutMesh.Material = MaterialPath;
}

static void ProcessScene(const aiNode* Node, ConvertContext Context, string OutDir)
{
	for (uint32 i = 0; i < Node->mNumMeshes; i++)
	{
		ProcessMesh(Context.ImportedScene->mMeshes[Node->mMeshes[i]], Context, OutDir);
	}

	for (uint32 i = 0; i < Node->mNumChildren; i++)
	{
		ProcessScene(Node->mChildren[i], Context, OutDir);
	}
}

static void WriteMaterial(string Path, ConvertContext Context, string Texture)
{
	using namespace graphics;

	Material* NewMaterial = Material::MakeDefault();

	auto TextureField = NewMaterial->FindField("u_texture", Material::Field::Type::Texture);

	if (TextureField)
	{
		TextureField->TextureValue.Name = new Material::MatTexture(Texture);
	}

	auto UseTextureField = NewMaterial->FindField("u_useTexture", Material::Field::Type::Bool);

	if (UseTextureField)
	{
		UseTextureField->Int = Texture.empty() ? 0 : 1;
	}

	NewMaterial->ToFile(Path);
	delete NewMaterial;
}

static void ProcessMaterials(const aiScene* Scene, ConvertContext Context, string OutDir)
{
	for (size_t i = 0; i < Scene->mNumMaterials; i++)
	{
		aiMaterial* material = Scene->mMaterials[i];
		aiString texture_file;
		material->Get(AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0), texture_file);
		string TexturePath;
		if (auto texture = Scene->GetEmbeddedTexture(texture_file.C_Str()))
		{
			string FileName;
			FileName = texture->mFilename.C_Str();
			if (FileName.empty())
			{
				FileName = Context.SceneName;
			}

			TexturePath = str::Format("%s_%i.png", FileName.c_str(), int(i));

			std::ofstream TextureOutput = std::ofstream(OutDir + TexturePath, std::ios::binary);
			if (!texture->mHeight)
			{
				TextureOutput.write((char*)texture->pcData, texture->mWidth);
			}
			TextureOutput.close();
		}

		string MaterialPath = str::Format("%s%s_%i.kmt", OutDir.c_str(), Context.SceneName.c_str(), int(i));
		WriteMaterial(MaterialPath, Context, TexturePath);
	}
}

static void ImportPrint(string Message)
{
	Log::Info(Message, { Log::LogPrefix{ .Text = "Import", .Color = Log::LogColor::Gray } });
}

static void ImportLogCallback(const char* msg, char* userData)
{
	ImportPrint(msg);
}

string engine::editor::modelConverter::ConvertModel(string ModelPath, string OutDir, ConvertOptions Options)
{
	if (OutDir.empty())
	{
		OutDir = "./";
	}

	if (OutDir[OutDir.size() - 1] != '/')
	{
		OutDir.push_back('/');
	}

	string Extension = ModelPath.substr(ModelPath.find_last_of(".") + 1);
	if (Extension == "fbx")
	{
		Options.ImportScale /= 100.0f;
	}

	ConvertContext ctx;
	ctx.SceneName = file::FileNameWithoutExt(ModelPath);
	ctx.Options = Options;

	aiLogStream stream;
	stream.callback = ImportLogCallback;
	aiAttachLogStream(&stream);

	ImportPrint("--- Starting import process ---");
	ImportPrint(
		str::Format("Optimize: %i, Generate UV: %i, Import Materials %i, Import Textures: %i, Import Scale: %f",
		int(Options.Optimize), int(Options.GenerateUV), int(Options.ImportMaterials), int(Options.ImportTextures), Options.ImportScale)
	);

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
		Options.OnLoadStatusChanged("Reading file (1/3)");
	}

	Assimp::Importer Import;
	ctx.ImportedScene = Import.ReadFile(ModelPath, 0);

	if (ctx.ImportedScene == nullptr)
	{
		aiDetachLogStream(&stream);
		return "";
	}

	ctx.Data = new ModelData();

	if (Options.OnLoadStatusChanged)
	{
		Options.OnLoadStatusChanged("Processing (2/3)");
	}

	Import.ApplyPostProcessing(Flags);

	ProcessScene(ctx.ImportedScene->mRootNode, ctx, OutDir);
	ProcessMaterials(ctx.ImportedScene, ctx, OutDir);

	if (Options.OnLoadStatusChanged)
	{
		Options.OnLoadStatusChanged("Saving (3/3)");
	}

	string OutFileName = file::FileNameWithoutExt(ModelPath) + ".kmdl";
	ImportPrint("Saving to file...");
	ctx.Data->ToFile(OutDir + OutFileName);
	aiDetachLogStream(&stream);
	ImportPrint("--- Import complete ---");

	delete ctx.Data;
	return OutDir + OutFileName;
}
#endif