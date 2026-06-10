#pragma once
#include <Core/Types.h>
#include <Core/File/SerializedData.h>
#include <unordered_map>
#include <optional>
#include <ds/typeId.hpp>

namespace engine::editor
{
	struct DocumentationFunctionArgument
	{
		string Name;
		string Description;
	};

	struct DocumentationFunction
	{
		string Description;
		std::vector<DocumentationFunctionArgument> Arguments;
		string ReturnValueDescription;
	};

	struct DocumentationMember
	{
		string Description;
	};

	struct DocumentationClasses
	{
		std::unordered_map<string, DocumentationMember*> Members;
	};

	class DocumentationDatabase
	{
	public:

		DocumentationDatabase() = default;
		DocumentationDatabase(const DocumentationDatabase&) = delete;
		~DocumentationDatabase();

		void Initialize(string EditorPath);

		void LoadDocumentationFile(string Path);

		DocumentationFunction* LoadFunction(SerializedValue& FromValue);
		DocumentationMember* LoadMember(SerializedValue& FromValue);

		DocumentationFunction* GetFunction(string FullName);
		DocumentationMember* GetTypeMember(string Type, string FullName);

		std::unordered_map<string, DocumentationFunction*> Functions;
		std::unordered_map<string, DocumentationClasses> Classes;
	};
}