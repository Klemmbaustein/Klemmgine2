#include "ModelData.h"
#include "Core/File/BinarySerializer.h"
#include <filesystem>
#include <mutex>
#include <Core/Log.h>
#include <Engine/MainThread.h>
#include <Engine/Scene.h>
#include <Engine/File/Resource.h>
#include <Engine/Graphics/VideoSubsystem.h>

using namespace engine;
using namespace engine::graphics;

std::mutex ModelDataMutex;
const engine::string FormatName = "kmdl";
std::unordered_map<engine::string, engine::GraphicsModel> engine::GraphicsModel::Models;

engine::ModelData::ModelData(string FilePath, bool LoadMaterials)
{
	this->LoadMaterials = LoadMaterials;

	std::vector<SerializedData> File;

	IBinaryStream* BinaryFile = resource::GetBinaryFile(FilePath);

	if (!BinaryFile)
		return;

	BinarySerializer::FromStream(BinaryFile, File, FormatName);
	DeSerialize(&File.at(0).Value);
	delete BinaryFile;
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

		if (!resource::FileExists(Asset.FilePath))
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

			auto NewAsset = AssetRef::Convert(f.TextureValue.Name->Name);

			if (!NewAsset.IsValid())
				continue;
			With->PreLoadAsset(NewAsset);
		}
	}
}

void engine::ModelData::ToFile(string FilePath)
{
	FileStream TargetFile = FileStream(FilePath, false);
	ToBinary(&TargetFile);
}

void engine::ModelData::ToBinary(IBinaryStream* To)
{
	BinarySerializer::ToBinaryData({ SerializedData("meshes", Serialize()) }, To, FormatName);
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
			Meshes.emplace_back(LoadMaterials).DeSerialize(&Elem);
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
		Mesh& NewMesh = Meshes.emplace_back(LoadMaterials);
		NewMesh.DeSerialize(&Elem);
		this->Bounds.Position += NewMesh.Bounds.Position;
		this->Bounds.Extents = Vector3(
			std::max(NewMesh.Bounds.Extents.X, Bounds.Extents.X),
			std::max(NewMesh.Bounds.Extents.Y, Bounds.Extents.Y),
			std::max(NewMesh.Bounds.Extents.Z, Bounds.Extents.Z));
	}
	this->Bounds.Position = this->Bounds.Position / float(MeshesArray.size());

	CastShadow = From->At("shadow").GetBool();
	HasCollision = From->At("collision").GetBool();
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
		SerializedData("mat", Material)
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
		{
			HasUV = i.Contains("uv");
			CheckedUV = true;
		}

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

	if (LoadMaterials && From->Contains("mat"))
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

engine::GraphicsModel* engine::GraphicsModel::RegisterModel(AssetRef Asset, bool LoadMaterials, bool Lock)
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
		New = new ModelData(Asset.FilePath, LoadMaterials);
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

engine::GraphicsModel* engine::GraphicsModel::GetModel(AssetRef Asset, bool LoadMaterials)
{
	std::lock_guard g{ ModelDataMutex };
	auto Found = Models.find(Asset.FilePath);
	if (Found == Models.end())
	{
		if (resource::FileExists(Asset.FilePath))
		{
			RegisterModel(Asset, LoadMaterials, false);
			Found = Models.find(Asset.FilePath);
			if (Found == Models.end())
				return nullptr;
		}
		else if (!Asset.FilePath.empty())
		{
			Log::Warn(str::Format("Failed to load model: %s", Asset.FilePath.c_str()));
			return nullptr;
		}
		else
		{
			return nullptr;
		}
	}
	else
	{
		Found->second.References++;
	}

	if (Found->second.Drawable == nullptr && VideoSubsystem::Current)
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
}

static void AddPlane(Vector3 Dir, Vector3 Offset, Vector3 Up, Vector3 Right, ModelData::Mesh& m)
{
	uint32 InitialLength = m.Vertices.size();
	m.Vertices.push_back(Vertex({
			.Position = Offset - Up - Right,
			.UV = Vector2(0, 0),
			.Normal = Dir,
		}));
	m.Vertices.push_back(Vertex({
			.Position = Offset - Up + Right,
			.UV = Vector2(1, 0),
			.Normal = Dir,
		}));
	m.Vertices.push_back(Vertex({
			.Position = Offset + Up - Right,
			.UV = Vector2(0, 1),
			.Normal = Dir,
		}));
	m.Vertices.push_back(Vertex({
			.Position = Offset + Up + Right,
			.UV = Vector2(1, 1),
			.Normal = Dir,
		}));

	m.Indices.push_back(InitialLength);
	m.Indices.push_back(InitialLength + 1);
	m.Indices.push_back(InitialLength + 2);
	m.Indices.push_back(InitialLength + 1);
	m.Indices.push_back(InitialLength + 3);
	m.Indices.push_back(InitialLength + 2);
}

engine::GraphicsModel* engine::GraphicsModel::UnitCube()
{
	static GraphicsModel* Cube = nullptr;

	if (!Cube)
	{
		Cube = new GraphicsModel();
		Cube->Data = new ModelData();

		auto& m = Cube->Data->Meshes.emplace_back(false);

		auto AddFace = [&](Vector3 Dir, Vector3 Up, Vector3 Right) {
			AddPlane(Dir, Dir, Up, Right, m);
		};

		AddFace(Vector3(0, 1, 0), Vector3(1, 0, 0), Vector3(0, 0, 1));
		AddFace(Vector3(0, -1, 0), Vector3(-1, 0, 0), Vector3(0, 0, 1));
		AddFace(Vector3(1, 0, 0), Vector3(0, -1, 0), Vector3(0, 0, 1));
		AddFace(Vector3(-1, 0, 0), Vector3(0, -1, 0), Vector3(0, 0, -1));
		AddFace(Vector3(0, 0, 1), Vector3(0, 1, 0), Vector3(1, 0, 0));
		AddFace(Vector3(0, 0, -1), Vector3(0, -1, 0), Vector3(1, 0, 0));

		Cube->Drawable = new graphics::Model(Cube->Data);
	}

	return Cube;
}

GraphicsModel* engine::GraphicsModel::UnitPlane()
{
	static GraphicsModel* Plane = nullptr;

	if (!Plane)
	{
		Plane = new GraphicsModel();
		Plane->Data = new ModelData();

		auto& m = Plane->Data->Meshes.emplace_back(false);

		AddPlane(Vector3(0, 1, 0), 0, Vector3(1, 0, 0), Vector3(0, 0, 1), m);
		AddPlane(Vector3(0, -1, 0), 0, Vector3(-1, 0, 0), Vector3(0, 0, 1), m);

		Plane->Drawable = new graphics::Model(Plane->Data);
	}

	return Plane;
}
