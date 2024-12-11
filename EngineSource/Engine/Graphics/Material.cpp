#include "Material.h"
#include "ShaderObject.h"
#include <GL/glew.h>
#include <kui/Image.h>
#include <kui/Resource.h>
#include <Engine/Log.h>
#include <Engine/File/TextSerializer.h>

using namespace engine;
using namespace engine::graphics;

engine::graphics::Material::Material(string FilePath)
{
	try
	{
		SerializedValue FileData = TextSerializer::FromFile(FilePath);
		DeSerialize(&FileData);
	}
	catch (SerializeReadException& ReadErr)
	{
		Log::Warn(ReadErr.what());
	}
	catch (SerializeException& Err)
	{
		Log::Warn(Err.what());
	}
}

engine::graphics::Material::Material()
{
}

engine::graphics::Material::~Material()
{
	Clear();
}

SerializedValue Material::Serialize()
{
	std::vector<SerializedData> Values;

	for (auto& i : Fields)
	{
		SerializedValue Val;
		switch (i.FieldType)
		{
		case Field::Type::Int:
			Val = i.Int;
			break;
		case Field::Type::Float:
			Val = i.Float;
			break;
		case Field::Type::Vec3:
			Val = i.Vec3;
			break;
		case Field::Type::Texture:
			Val = *i.Texture.Name;
			break;
		case Field::Type::None:
		default:
			break;
		}
		Values.push_back(SerializedData(i.Name,
			std::vector{
				SerializedData("type", int32(i.FieldType)),
				SerializedData("val", Val),
			}
			));
	}

	Values.push_back(SerializedData("vertexShader",
		std::vector{
			SerializedData("type", int32(-1)),
			SerializedData("val", VertexShader),
		}
		));
	Values.push_back(SerializedData("fragmentShader",
		std::vector{
			SerializedData("type", int32(-1)),
			SerializedData("val", FragmentShader),
		}
		));

	return Values;
}

void Material::DeSerialize(SerializedValue* From)
{
	Clear();
	std::vector<SerializedData>& Values = From->GetObject();
	for (SerializedData& i : Values)
	{
		if (i.Value.GetType() != SerializedData::DataType::Object)
		{
			if (i.Name == "vertexShader")
			{
				VertexShader = i.Value.GetString();
				continue;
			}
			if (i.Name == "fragmentShader")
			{
				FragmentShader = i.Value.GetString();
				continue;
			}
		}

		Field::Type Type = Field::Type(i.At("type").GetInt());
		SerializedValue Val = i.At("val");

		Field NewField;
		NewField.Name = i.Name;
		NewField.FieldType = Type;
		switch (Type)
		{
		case Field::Type::Int:
			NewField.Int = Val.GetInt();
			break;
		case Field::Type::Float:
			NewField.Float = Val.GetFloat();
			break;
		case Field::Type::Vec3:
			NewField.Vec3 = Val.GetVector3();
			break;
		case Field::Type::Texture:
			NewField.Texture.Name = new std::string(Val.GetString());
			NewField.Texture.Value = 0;
			break;
		case Field::Type::None:
		default:
			break;
		}
		Fields.push_back(NewField);
	}
	Shader = new ShaderObject(
		{ kui::resource::GetStringFile("res:shader/" + VertexShader) },
		{ kui::resource::GetStringFile("res:shader/" + FragmentShader) }
	);
}

void Material::Clear()
{
	for (const Field& i : Fields)
	{
		if (i.FieldType == Field::Type::Texture && i.Texture.Name)
		{
			delete i.Texture.Name;
		}
	}
	Fields.clear();

	if (Shader)
		delete Shader;
}

void engine::graphics::Material::Apply()
{
	if (!Shader)
		return;
	Shader->Bind();
	uint8 TextureSlot = 4;
	for (auto& i : Fields)
	{
		if (i.UniformLocation == 0)
		{
			i.UniformLocation = Shader->GetUniformLocation(i.Name);
		}
		switch (i.FieldType)
		{
		case Field::Type::Int:
			Shader->SetInt(i.UniformLocation, i.Int);
			break;
		case Field::Type::Float:
			Shader->SetFloat(i.UniformLocation, i.Float);
			break;
		case Field::Type::Vec3:
			Shader->SetVec3(i.UniformLocation, i.Vec3);
			break;
		case Field::Type::Texture:
		{
			if (i.Texture.Name && !i.Texture.Value)
			{
				// TODO: replace with engine's texture functions
				i.Texture.Value = kui::image::LoadImage(*i.Texture.Name);
			}
			glActiveTexture(GL_TEXTURE0 + TextureSlot);
			glBindTexture(GL_TEXTURE_2D, i.Texture.Value);
			Shader->SetInt(i.UniformLocation, TextureSlot);
			TextureSlot++;
			break;
		}
		case Field::Type::None:
		default:
			break;
		}
	}
}
