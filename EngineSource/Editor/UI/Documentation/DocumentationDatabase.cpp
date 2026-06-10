#include "DocumentationDatabase.h"
#include <Core/File/JsonSerializer.h>
#include <Core/Log.h>
#include <filesystem>

using namespace engine::editor;

engine::editor::DocumentationDatabase::~DocumentationDatabase()
{
	for (auto& i : this->Functions)
	{
		delete i.second;
	}
}

void engine::editor::DocumentationDatabase::Initialize(string EditorPath)
{
	try
	{
		for (auto& i : std::filesystem::directory_iterator(EditorPath + "/Editor/Documentation"))
		{
			LoadDocumentationFile(i.path().string());
		}
	}
	catch (std::exception& e)
	{
		Log::Warn(str::Format("Failed to load documentation files: %s", e.what()));
	}
	catch (SerializeException& e)
	{
		Log::Warn(str::Format("Failed to load documentation files: %s", e.what()));
	}
}

void engine::editor::DocumentationDatabase::LoadDocumentationFile(string Path)
{
	auto Loaded = JsonSerializer::FromFile(Path);

	if (Loaded.GetType() != SerializedData::DataType::Object)
	{
		return;
	}

	string CurrentModule = "";

	if (Loaded.Contains("module"))
	{
		CurrentModule = Loaded.At("module").GetString();
	}

	if (Loaded.At("type").GetString() != "bindings")
	{
		Log::Warn(str::Format("The documentation file %s has an unknown type.", Path.c_str()));
		return;
	}

	if (Loaded.Contains("functions"))
	{
		for (auto& i : Loaded.At("functions").GetObject())
		{
			this->Functions.insert({ CurrentModule + "::" + i.Name, LoadFunction(i.Value)});
		}
	}

	if (Loaded.Contains("types"))
	{
		for (auto& Type : Loaded.At("types").GetObject())
		{
			if (Type.Value.Contains("methods"))
			{
				for (auto& i : Type.Value.At("methods").GetObject())
				{
					this->Functions.insert({ CurrentModule + "::" + Type.Name + "." + i.Name, LoadFunction(i.Value) });
				}
			}
			if (Type.Value.Contains("members"))
			{
				for (auto& i : Type.Value.At("members").GetObject())
				{
					this->Classes[CurrentModule + "::" + Type.Name]
						.Members.insert({ i.Name, LoadMember(i.Value) });
				}
			}
		}
	}
}

DocumentationFunction* engine::editor::DocumentationDatabase::GetFunction(string FullName)
{
	auto found = this->Functions.find(FullName);

	if (found != this->Functions.end())
	{
		return found->second;
	}

	return nullptr;
}

DocumentationMember* engine::editor::DocumentationDatabase::GetTypeMember(string Type, string FullName)
{
	auto found = this->Classes.find(Type);

	if (found != this->Classes.end())
	{
		auto member = found->second.Members.find(FullName);

		if (member != found->second.Members.end())
		{
			return member->second;
		}
	}

	return nullptr;
}

DocumentationFunction* engine::editor::DocumentationDatabase::LoadFunction(SerializedValue& FromValue)
{
	DocumentationFunction* Function = new DocumentationFunction();

	if (FromValue.Contains("description"))
	{
		Function->Description = FromValue.At("description").GetString();
	}

	if (FromValue.Contains("arguments"))
	{
		for (auto& i : FromValue.At("arguments").GetObject())
		{
			Function->Arguments.push_back(DocumentationFunctionArgument{
				.Name = i.Name,
				.Description = i.Value.GetString()
				});
		}
	}

	if (FromValue.Contains("return"))
	{
		Function->ReturnValueDescription = FromValue.At("return").GetString();
	}

	return Function;
}

DocumentationMember* engine::editor::DocumentationDatabase::LoadMember(SerializedValue& FromValue)
{
	DocumentationMember* Member = new DocumentationMember();

	if (FromValue.Contains("description"))
	{
		Member->Description = FromValue.At("description").GetString();
	}

	return Member;
}