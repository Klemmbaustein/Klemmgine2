#include "Material.h"
#include "Material.h"
#include "ShaderLoader.h"
#include <Core/Log.h>
#include <Engine/File/Resource.h>
#include <Core/File/TextSerializer.h>
#include <Core/File/BinarySerializer.h>
#include <Engine/Internal/OpenGL.h>
#include <Engine/MainThread.h>
#include <Engine/File/Resource.h>

using namespace engine;
using namespace engine::graphics;

engine::graphics::Material::Material(AssetRef File)
{
	try
	{
		SerializedValue FileData;

		if (File.Extension == "kmt")
		{
			FileData = TextSerializer::FromFile(File.FilePath);
		}
		else
		{
			IBinaryStream* BinaryFile = resource::GetBinaryFile(File.FilePath);
			FileData = BinarySerializer::FromStream(BinaryFile, "kbm");
			delete BinaryFile;
		}

		if (!FileData.GetObject().empty())
		{
			DeSerialize(&FileData);
			return;
		}
		else
		{
			SetToDefault();
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

	if (!File.FilePath.empty())
	{
		Log::Warn(str::Format("Failed to load material file: '%s'", File.FilePath.c_str()));
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
		case Field::Type::Bool:
			Val = SerializedValue(bool(i.Int));
			break;
		case Field::Type::Float:
			Val = i.Float;
			break;
		case Field::Type::Vec3:
			Val = i.Vec3;
			break;
		case Field::Type::Texture:
			if (i.TextureValue.Name)
			{
				Val = SerializedValue({
					SerializedData("file", i.TextureValue.Name->Name),
					SerializedData("filter", i.TextureValue.Name->Options.Filter),
					SerializedData("border", i.TextureValue.Name->Options.TextureBorders),
					});
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
	size_t it = 0;
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

		Field& NewField = Fields.emplace_back();
		NewField.Name = i.Name;
		NewField.FieldType = Type;
		switch (Type)
		{
		case Field::Type::Bool:
			NewField.Int = Val.GetBool();
			break;
		case Field::Type::Int:
			NewField.Int = Val.GetInt();
			if (i.Name == "u_useTexture")
			{
				UseTexture = NewField.Int;
			}
			break;
		case Field::Type::Float:
			NewField.Float = Val.GetFloat();
			break;
		case Field::Type::Vec3:
			NewField.Vec3 = Val.GetVector3();
			break;
		case Field::Type::Texture:
		{
			if (Val.GetType() == SerializedData::DataType::String)
			{
				NewField.TextureValue.Name = new MatTexture(Val.GetString());
			}
			else
			{
				TextureOptions Options;
				Options.Filter = TextureOptions::Filtering(Val.At("filter").GetByte());
				Options.TextureBorders = TextureOptions::BorderType(Val.At("border").GetByte());

				NewField.TextureValue.Name = new MatTexture(
					Val.At("file").GetString(),
					Options);
			}
			NewField.TextureValue.Value = 0;
			if (i.Name == "u_texture")
			{
				TextureField = it;
			}
			break;
		}
		case Field::Type::None:
		default:
			break;
		}
		it++;
	}

	UpdateShader();
}

void engine::graphics::Material::LoadTexture(Field& From)
{
	if (From.TextureValue.Name && !From.TextureValue.Value)
	{
		From.TextureValue.Value = TextureLoader::Instance->LoadTextureFile(
			AssetRef::Convert(From.TextureValue.Name->Name),
			From.TextureValue.Name->Options
		);
	}
}

void Material::Clear()
{
	for (const Field& i : Fields)
	{
		if (i.FieldType == Field::Type::Texture && i.TextureValue.Name)
		{
			delete i.TextureValue.Name;
		}
	}
	Fields.clear();

	Shader = nullptr;
	TextureField = SIZE_MAX;
	UseTexture = false;
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
		case Field::Type::Bool:
			if (i.Name == "u_useTexture")
			{
				UseTexture = i.Int;
			}
			Shader->SetInt(i.UniformLocation, i.Int);
			break;
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
			if (!i.TextureValue.Value)
				LoadTexture(i);
			glActiveTexture(GL_TEXTURE1 + TextureSlot);
			if (i.TextureValue.Value)
			{
				glBindTexture(GL_TEXTURE_2D, i.TextureValue.Value->TextureObject);
			}
			else
			{
				glBindTexture(GL_TEXTURE_2D, 0);
			}
			Shader->SetInt(i.UniformLocation, TextureSlot + 1);
			TextureSlot++;
			break;
		}
		case Field::Type::None:
		default:
			break;
		}
	}
}

void engine::graphics::Material::ApplySimple(graphics::ShaderObject* With)
{
	With->SetInt(With->GetUniformLocation("u_useTexture"), UseTexture && TextureField != SIZE_MAX);

	if (TextureField != SIZE_MAX && UseTexture)
	{
		auto& tx = Fields[TextureField];

		if (tx.FieldType != Field::Type::Texture)
			return;

		if (!tx.TextureValue.Value)
			LoadTexture(tx);
		glActiveTexture(GL_TEXTURE1);
		if (tx.TextureValue.Value)
		{
			glBindTexture(GL_TEXTURE_2D, tx.TextureValue.Value->TextureObject);
		}
		else
		{
			glBindTexture(GL_TEXTURE_2D, 0);
		}
		With->SetInt(With->GetUniformLocation("u_texture"), 1);
	}
}

void engine::graphics::Material::VerifyUniforms()
{
	auto Result = ShaderLoader::Current->Modules.ParseShader(resource::GetTextFile(FragmentShader), ShaderModule::ShaderType::Fragment);

	bool FoundTexture = false;
	std::vector<Field> NewFields;

	for (auto& i : Result.ShaderUniforms)
	{
		auto Found = FindField(i.Name, i.Type);
		if (Found)
		{
			if (i.Name == "u_texture")
			{
				FoundTexture = true;
				TextureField = NewFields.size();
			}
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
			case Field::Type::Bool:
				NewField.Int = i.DefaultValue == "true" ? 1 : 0;
				break;
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
			NewField.TextureValue.Name = nullptr;
			NewField.TextureValue.Value = nullptr;
		}

		NewFields.push_back(NewField);
	}
	if (!FoundTexture)
	{
		TextureField = SIZE_MAX;
		UseTexture = false;
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
	UseTextureField.FieldType = Field::Type::Bool;
	UseTextureField.Int = 0;

	Field ColorField;

	ColorField.Name = "u_color";
	ColorField.FieldType = Field::Type::Vec3;
	ColorField.Vec3 = 1;

	Fields = {
		UseTextureField,
		ColorField,
	};
	IsDefault = true;

	VerifyUniforms();
	UpdateShader();
}

void engine::graphics::Material::UpdateShader()
{
	if (!thread::IsMainThread)
		return;

	Shader = ShaderLoader::Current->Get(
		VertexShader,
		FragmentShader
	);

	// Don't try to use an invalid shader.
	if (!Shader->Valid)
	{
		Log::Error("Shader is not valid!");
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
