#include "Material.h"
#include "ShaderLoader.h"
#include <Engine/Log.h>
#include <Engine/File/Resource.h>
#include <Engine/File/TextSerializer.h>
#include <Engine/Internal/OpenGL.h>

using namespace engine;
using namespace engine::graphics;

engine::graphics::Material::Material(string FilePath)
{
	try
	{
		SerializedValue FileData = TextSerializer::FromFile(FilePath);
		if (!FileData.GetObject().empty())
		{
			DeSerialize(&FileData);
			return;
		}
	}
	catch (SerializeReadException& ReadErr)
	{
		Log::Warn(ReadErr.what());
	}
	catch (SerializeException& Err)
	{
		Log::Warn(Err.what());
	}
	SetToDefault();
}

Material* engine::graphics::Material::MakeDefault()
{
	auto* Result = new Material();

	Result->SetToDefault();

	return Result;
}

engine::graphics::Material::Material()
{
}

engine::graphics::Material::~Material()
{
	Clear();
}

void engine::graphics::Material::ToFile(string Path)
{
	TextSerializer::ToFile(Serialize().GetObject(), Path);
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
			if (i.Texture.Name)
			{
				Val = *i.Texture.Name;
			}
			else
			{
				Val = "";
			}
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
		VertexShader
	));
	Values.push_back(SerializedData("fragmentShader",
		FragmentShader
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

	UpdateShader();
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

	Shader = nullptr;
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
				i.Texture.Value = TextureLoader::Instance->LoadTextureFile(AssetRef::FromPath(*i.Texture.Name), TextureLoadOptions{});
			}
			glActiveTexture(GL_TEXTURE0 + TextureSlot);
			if (i.Texture.Value)
			{
				glBindTexture(GL_TEXTURE_2D, i.Texture.Value->TextureObject);
			}
			else
			{
				glBindTexture(GL_TEXTURE_2D, 0);
			}
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

void engine::graphics::Material::VerifyUniforms()
{
	auto Result = ShaderLoader::Current->Modules.ParseShader(resource::GetTextFile(FragmentShader));

	std::vector<Field> NewFields;

	for (auto& i : Result.ShaderUniforms)
	{
		auto Found = FindField(i.Name, i.Type);
		if (Found)
		{
			NewFields.push_back(*Found);
			continue;
		}
		Field NewField;
		NewField.Name = i.Name;
		NewField.FieldType = i.Type;
		if (i.DefaultValue != "")
		{
			switch (NewField.FieldType)
			{
			case Field::Type::Int:
				NewField.Int = std::stoi(i.DefaultValue);
				break;
			case Field::Type::Float:
				NewField.Float = std::stof(i.DefaultValue);
				break;
			case Field::Type::Vec3:
				NewField.Vec3 = Vector3::FromString(i.DefaultValue);
				break;
			case Field::Type::None:
			default:
				break;
			}
		}
		else if (NewField.FieldType == Field::Type::Texture)
		{
			NewField.Texture.Name = nullptr;
			NewField.Texture.Value = 0;
		}

		NewFields.push_back(NewField);
	}
	Fields = NewFields;
}

Material::Field* engine::graphics::Material::FindField(string Name, Field::Type Type)
{
	for (auto& i : Fields)
	{
		if (i.Name == Name && i.FieldType == Type)
		{
			return &i;
		}
	}
	return nullptr;
}

void engine::graphics::Material::SetToDefault()
{
	VertexShader = "res:shader/basic.vert";
	FragmentShader = "res:shader/basic.frag";

	Field UseTextureField;

	UseTextureField.Name = "u_useTexture";
	UseTextureField.FieldType = Field::Type::Int;
	UseTextureField.Int = 0;

	Field ColorField;

	UseTextureField.Name = "u_color";
	UseTextureField.FieldType = Field::Type::Vec3;
	UseTextureField.Vec3 = 1;

	Fields = {
		UseTextureField,
		ColorField,
	};
	IsDefault = true;

	UpdateShader();
}

void engine::graphics::Material::UpdateShader()
{
	Shader = ShaderLoader::Current->Get(
		VertexShader,
		FragmentShader
	);

	// Don't try to use an invalid shader.
	if (!Shader->Valid)
	{
		// Default shader isn't valid, oh no!
		if (IsDefault)
		{
			Log::Error("Default shader is not valid!");
			Shader = nullptr;
		}
		else
		{
			SetToDefault();
		}
	}

#if EDITOR || DEBUG
	VerifyUniforms();
#endif
}
