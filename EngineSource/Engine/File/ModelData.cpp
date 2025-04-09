#include "ModelData.h"
#include "Core/File/BinarySerializer.h"
#include "Core/File/FileUtil.h"
#include <filesystem>
#include <mutex>
#include <Core/Log.h>
#include <Engine/MainThread.h>
#include <Engine/Scene.h>
#include <Engine/File/Resource.h>
#include <Engine/Subsystem/VideoSubsystem.h>

using namespace engine::graphics;

std::mutex ModelDataMutex;
const engine::string FormatName = "kmdl";
std::unordered_map<engine::string, engine::GraphicsModel> engine::GraphicsModel::Models;

engine::ModelData::ModelData(string FilePath)
{
	std::vector<SerializedData> File;

	IBinaryStream* BinaryFile = resource::GetBinaryFile(FilePath);

	if (!BinaryFile)
		return;

	BinarySerializer::FromStream(BinaryFile, File, FormatName);
	DeSerialize(&File.at(0).Value);
}

engine::ModelData::ModelData()
{
}

engine::ModelData::~ModelData()
{
}

void engine::ModelData::PreLoadMaterials(Scene* With)
{
	for (auto& i : Meshes)
	{
		auto Asset = AssetRef::FromName(i.Material, "kmt");

		if (!Asset.Exists())
		{
			Asset = AssetRef::FromName(i.Material, "kbm");
		}

		Material Mat = graphics::Material(Asset);
		for (auto& f : Mat.Fields)
		{
			if (f.FieldType != Material::Field::Type::Texture)
			{
				continue;
			}

			if (f.TextureValue.Name == nullptr)
			{
				continue;
			}

			auto Asset = AssetRef::Convert(f.TextureValue.Name->Name);

			if (!Asset.IsValid())
				continue;
			With->PreLoadAsset(Asset);
		}
	}
}

void engine::ModelData::ToFile(string FilePath)
{
	BinarySerializer::ToFile({ SerializedData("meshes", Serialize()) }, FilePath, FormatName);
}

engine::SerializedValue engine::ModelData::Serialize()
{
	std::vector<SerializedValue> OutMeshes;

	for (auto& i : Meshes)
	{
		OutMeshes.push_back(i.Serialize());
	}

	return SerializedValue({
		SerializedData("meshes", OutMeshes),
		SerializedData("shadow", SerializedValue(CastShadow)),
		SerializedData("collision", SerializedValue(HasCollision)),
		});
}

void engine::ModelData::DeSerialize(SerializedValue* From)
{
	if (From->GetType() == SerializedData::DataType::Array)
	{
		auto& Array = From->GetArray();

		for (SerializedValue& Elem : Array)
		{
			Meshes.emplace_back().DeSerialize(&Elem);
		}
		return;
	}
	else if (From->GetType() != SerializedData::DataType::Object)
	{
		return;
	}

	auto& MeshesArray = From->At("meshes").GetArray();

	for (SerializedValue& Elem : MeshesArray)
	{
		Mesh& NewMesh = Meshes.emplace_back();
		NewMesh.DeSerialize(&Elem);
		this->Bounds.Position += NewMesh.Bounds.Position;
		this->Bounds.Extents = Vector3(
			std::max(NewMesh.Bounds.Extents.X, Bounds.Extents.X),
			std::max(NewMesh.Bounds.Extents.Y, Bounds.Extents.Y),
			std::max(NewMesh.Bounds.Extents.Z, Bounds.Extents.Z));
	}
	this->Bounds.Position = this->Bounds.Position / MeshesArray.size();

	CastShadow = From->At("shadow").GetBool();
	HasCollision = From->At("collision").GetBool();
}

engine::ModelData::Mesh::Mesh()
{
}

engine::ModelData::Mesh::Mesh(const std::vector<Vertex>& Vertices, const std::vector<uint32>& Indices)
{
	this->Vertices = Vertices;
	this->Indices = Indices;
}

engine::SerializedValue engine::ModelData::Mesh::Serialize()
{
	std::vector<SerializedValue> OutVertices;
	std::vector<SerializedValue> OutIndices;
	OutVertices.reserve(Vertices.size());
	for (const Vertex& v : Vertices)
	{
		OutVertices.push_back(SerializedValue({
			SerializedData("ps", v.Position),
			SerializedData("nm", v.Normal),
			SerializedData("uv", SerializedValue(v.UV)),
			}));
	}

	OutIndices.reserve(Indices.size());
	for (uint32 i : Indices)
	{
		OutIndices.push_back(int32(i));
	}

	return std::vector{
		SerializedData("vrt", OutVertices),
		SerializedData("ind", OutIndices),
		SerializedData("mat", file::FileNameWithoutExt(Material))
	};
}

void engine::ModelData::Mesh::DeSerialize(SerializedValue* From)
{
	SerializedValue& InVertices = From->At("vrt");
	SerializedValue& InIndices = From->At("ind");

	bool CheckedUV = false;
	bool HasUV = false;

	Vertices.reserve(InVertices.GetArray().size());
	Vector3 BoundsMin, BoundsMax;
	for (auto& i : InVertices.GetArray())
	{
		if (!CheckedUV)
			HasUV = i.Contains("uv");

		if (HasUV)
		{
			auto PosVec = i.At("ps").GetVector3();

			BoundsMin = Vector3(
				std::min(PosVec.X, BoundsMin.X),
				std::min(PosVec.Y, BoundsMin.Y),
				std::min(PosVec.Z, BoundsMin.Z));
			BoundsMax = Vector3(
				std::max(PosVec.X, BoundsMax.X),
				std::max(PosVec.Y, BoundsMax.Y),
				std::max(PosVec.Z, BoundsMax.Z));

			Vertices.emplace_back(PosVec, i.At("uv").GetVector2(), i.At("nm").GetVector3());
		}
		else
		{
			Vertices.emplace_back(i.At("ps").GetVector3(), Vector2(-1), i.At("nm").GetVector3());
		}
	}

	this->Bounds = BoundingBox{
		.Position = (BoundsMin + BoundsMax) / 2,
		.Extents = BoundsMax - BoundsMin
	};

	Indices.reserve(InIndices.GetArray().size());
	for (auto& i : InIndices.GetArray())
	{
		Indices.push_back(i.GetInt());
	}

	if (From->Contains("mat"))
	{
		this->Material = From->At("mat").GetString();
	}
}

engine::GraphicsModel* engine::GraphicsModel::RegisterModel(const ModelData& Mesh, string Name, bool Lock)
{
	if (Lock)
		ModelDataMutex.lock();

	GraphicsModel* New = nullptr;

	if (thread::IsMainThread)
	{
		New = &Models.insert({ Name, GraphicsModel{
			.Data = new ModelData(Mesh),
			.Drawable = new Model(&Mesh),
			.References = 1,
			} }).first->second;
	}
	else
	{
		New = &Models.insert({ Name, GraphicsModel{
			.Data = new ModelData(Mesh),
			.Drawable = nullptr,
			.References = 1,
			} }).first->second;
	}
	if (Lock)
		ModelDataMutex.unlock();

	return New;
}

engine::GraphicsModel* engine::GraphicsModel::RegisterModel(AssetRef Asset, bool Lock)
{
	if (Lock)
		ModelDataMutex.lock();

	auto Found = Models.find(Asset.FilePath);
	if (Found != Models.end())
	{
		Found->second.References++;
		if (Lock)
			ModelDataMutex.unlock();
		return &Found->second;
	}
	if (Lock)
		ModelDataMutex.unlock();
	ModelData* New = nullptr;
	GraphicsModel* NewModel = nullptr;
	try
	{
		New = new ModelData(Asset.FilePath);
	}
	catch (SerializeReadException& e)
	{
		Log::Warn(str::Format("Failed to load model: %s", e.what()));
		return nullptr;
	}
	if (Lock)
		ModelDataMutex.lock();

	if (thread::IsMainThread)
	{
		NewModel = &Models.insert({ Asset.FilePath, GraphicsModel{
			.Data = New,
			.Drawable = new Model(New),
			.References = 1,
			} }).first->second;
	}
	else
	{
		NewModel = &Models.insert({ Asset.FilePath, GraphicsModel{
			.Data = New,
			.Drawable = nullptr,
			.References = 1,
			} }).first->second;
	}
	if (Lock)
		ModelDataMutex.unlock();

	return NewModel;
}

engine::GraphicsModel* engine::GraphicsModel::GetModel(AssetRef Asset)
{
	std::lock_guard g{ ModelDataMutex };
	auto Found = Models.find(Asset.FilePath);
	if (Found == Models.end())
	{
		if (std::filesystem::exists(Asset.FilePath))
		{
			RegisterModel(Asset, false);
			Found = Models.find(Asset.FilePath);
			if (Found == Models.end())
				return nullptr;
		}
		else
			return nullptr;
	}
	else
	{
		Found->second.References++;
	}

	if (Found->second.Drawable == nullptr && subsystem::VideoSubsystem::Current)
	{
		Found->second.Drawable = new Model(Found->second.Data);
	}

	return &Found->second;
}

void engine::GraphicsModel::ReferenceModel(GraphicsModel* Target)
{
	for (auto& i : Models)
	{
		if (&i.second != Target)
		{
			continue;
		}
		i.second.References++;
		return;
	}
	Log::Warn("Failed to reference model");
}

void engine::GraphicsModel::UnloadModel(GraphicsModel* Target)
{
	for (auto& i : Models)
	{
		if (&i.second != Target)
		{
			continue;
		}
		i.second.References--;
		if (i.second.References == 0)
		{
			delete i.second.Data;
			delete i.second.Drawable;
			for (auto& fn : i.second.OnDereferenced)
			{
				fn.second();
			}
			Target->Data = nullptr;
			Target->Drawable = nullptr;
			Models.erase(i.first);
		}
		return;
	}
	Log::Warn("Failed to unload model");
}

void engine::GraphicsModel::UnloadModel(AssetRef Asset)
{
	for (auto& i : Models)
	{
		if (i.first != Asset.FilePath)
		{
			continue;
		}
		i.second.References--;
		if (i.second.References == 0)
		{
			delete i.second.Data;
			delete i.second.Drawable;
			for (auto& fn : i.second.OnDereferenced)
			{
				fn.second();
			}
			i.second.Data = nullptr;
			i.second.Drawable = nullptr;
			Models.erase(i.first);
		}
		return;
	}
	Log::Warn("Failed to unload model");
}
