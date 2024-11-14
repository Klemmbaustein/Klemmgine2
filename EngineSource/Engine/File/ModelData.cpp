#include "ModelData.h"
#include "BinarySerializer.h"
#include <filesystem>

const engine::string FormatName = "kmdl";
std::unordered_map<engine::string, engine::GraphicsModel> engine::GraphicsModel::Models;

engine::ModelData::ModelData(string FilePath)
{
	auto File = BinarySerializer::FromFile(FilePath, FormatName);
	DeSerialize(&File.at(0).Value);
}

engine::ModelData::ModelData()
{
}

void engine::ModelData::ToFile(string FilePath)
{
	BinarySerializer::ToFile({ SerializedData("meshes", Serialize())}, FilePath, FormatName);
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
		Meshes.emplace_back();
		Meshes[Meshes.size() - 1].DeSerialize(&Elem);
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
			SerializedData("nm", v.Normal)
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
	};
}

void engine::ModelData::Mesh::DeSerialize(SerializedValue* From)
{
	SerializedValue& InVertices = From->At("vrt");
	SerializedValue& InIndices = From->At("ind");

	for (auto& i : InVertices.GetArray())
	{
		Vertices.emplace_back(i.At("ps").GetVector3(), i.At("nm").GetVector3());
	}

	for (auto& i : InIndices.GetArray())
	{
		Indices.push_back(i.GetInt());
	}
}

void engine::GraphicsModel::RegisterModel(const ModelData& Mesh, string Name)
{
	Models.insert({ Name, GraphicsModel{
		.Data = new ModelData(Mesh),
		.Drawable = new Model(&Mesh),
		.References = 1,
		} });
}

void engine::GraphicsModel::RegisterModel(string AssetPath)
{
	ModelData* New = new ModelData(AssetPath);

	Models.insert({ AssetPath, GraphicsModel{
		.Data = New,
		.Drawable = new Model(New),
		.References = 1,
		} });
}

engine::GraphicsModel* engine::GraphicsModel::GetModel(string NameOrPath)
{
	auto Found = Models.find(NameOrPath);
	if (Found == Models.end())
	{
		if (std::filesystem::exists(NameOrPath))
		{
			RegisterModel(NameOrPath);
			Found = Models.find(NameOrPath);
		}
		else
			return nullptr;
	}
	return &Found->second;
}

void engine::GraphicsModel::UnloadModel(ModelData* Target)
{
}
