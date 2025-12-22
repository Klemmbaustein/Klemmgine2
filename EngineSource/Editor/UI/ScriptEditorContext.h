#pragma once
#include <ds/language.hpp>
#include <kui/UI/TextEditor.h>
#include <Core/Event.h>

namespace engine::editor
{
	struct ScriptError
	{
		kui::EditorPosition At;
		size_t Length = 0;
		std::string Description;
	};

	class ScriptEditorContext
	{
	public:
		ds::LanguageService* ScriptService = nullptr;
		std::map<std::string, std::vector<ScriptError>> Errors;

		ScriptEditorContext();

		void AddFile(const std::string& Content, const std::string& Name);
		void UpdateFile(const std::string& Content, const std::string& Name);
		void Commit();

		void Initialize();

		bool Loaded = false;

		Event<> OnReady;
	};
}