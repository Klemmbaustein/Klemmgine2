#include "ModelData.h"
#include "BinarySerializer.h"
#include <filesystem>
#include <mutex>
#include <Engine/Log.h>
#include <Engine/MainThread.h>

using namespace engine::graphics;

std::mutex ModelDataMutex;
const engine::string FormatName = "kmdl";
std::unordered_map<engine::string, engine::GraphicsModel> engine::GraphicsModel::Models;

engine::ModelData::ModelData(string FilePath)
{
	std::vector<SerializedData> File;
	BinarySerializer::FromFile(FilePath, FormatName, File);
	DeSerialize(&File.at(0).Value);
}

engine::ModelData::ModelData()
{
}

engine::ModelData::~ModelData()
{
}

void engine::ModelData::ToFile(string FilePath)
{
	BinarySerializer::ToFile({ SerializedData("meshes", Serialize()) }, FilePath, FormatName);
}

engine::SerializedValue engine::ModelData::Serialize()
{
	std::vector<SerializedValue> Out;

	for (auto& i : Meshes)
	{
		Out.push_back(i.Serialize());
	}

	return Out;
}

void engine::ModelData::DeSerialize(SerializedValue* From)
{
	if (From->GetType() != SerializedData::DataType::Array)
	{
		return;
	}

	auto& Array = From->GetArray();

	for (SerializedValue& Elem : Array)
	{
		Meshes.emplace_back().DeSerialize(&Elem);
	}
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
	for (auto& i : InVertices.GetArray())
	{
		if (!CheckedUV)
			HasUV = i.Contains("uv");

		if (HasUV)
		{
			Vertices.emplace_back(i.At("ps").GetVector3(), i.At("uv").GetVector2(), i.At("nm").GetVector3());
		}
		else
		{
			Vertices.emplace_back(i.At("ps").GetVector3(), Vector2(-1), i.At("nm").GetVector3());
		}
	}

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

void engine::GraphicsModel::RegisterModel(const ModelData& Mesh, string Name, bool Lock)
{
	if (Lock)
		ModelDataMutex.lock();

	if (thread::IsMainThread)
	{
		Models.insert({ Name, GraphicsModel{
			.Data = new ModelData(Mesh),
			.Drawable = new Model(&Mesh),
			.References = 1,
			} });
	}
	else
	{
		Models.insert({ Name, GraphicsModel{
			.Data = new ModelData(Mesh),
			.Drawable = nullptr,
			.References = 1,
			} });
	}
	if (Lock)
		ModelDataMutex.unlock();
}

void engine::GraphicsModel::RegisterModel(AssetRef Asset, bool Lock)
{
	if (Lock)
		ModelDataMutex.lock();

	if (Models.contains(Asset.FilePath))
	{
		Models[Asset.FilePath].References++;
		if (Lock)
			ModelDataMutex.unlock();
		return;
	}
	if (Lock)
		ModelDataMutex.unlock();
	ModelData* New = nullptr;
	try
	{
		New = new ModelData(Asset.FilePath);
	}
	catch (SerializeReadException& e)
	{
		Log::Warn(str::Format("Failed to load model: %s", e.what()));
		return;
	}
	if (Lock)
		ModelDataMutex.lock();

	if (thread::IsMainThread)
	{
		Models.insert({ Asset.FilePath, GraphicsModel{
			.Data = New,
			.Drawable = new Model(New),
			.References = 1,
			} });
	}
	else
	{
		Models.insert({ Asset.FilePath, GraphicsModel{
			.Data = New,
			.Drawable = nullptr,
			.References = 1,
			} });
	}
	if (Lock)
		ModelDataMutex.unlock();

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

	if (Found->second.Drawable == nullptr)
	{
		Found->second.Drawable = new Model(Found->second.Data);
	}

	return &Found->second;
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
			i.second.Data = nullptr;
			i.second.Drawable = nullptr;
			Models.erase(i.first);
		}
		return;
	}
	Log::Warn("Failed to unload model");
}
