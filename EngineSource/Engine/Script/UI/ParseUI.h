#pragma once
#include <ds/parser/parser.hpp>
#include <Core/Types.h>
#include <Markup/MarkupParse.h>

namespace engine::script::ui
{
	struct ClassMapping
	{
		kui::markup::MarkupElement Element;
	};

	struct UIParseData
	{
		kui::markup::ParseResult UIData;
		std::map<ds::TypeId, string> ClassIdMappings;

	};

	class UIFileParser
	{
	public:
		UIFileParser();

		void AddFile(string Path);
		void AddString(string Path, const string& Content);

		UIParseData Parse(ds::ParseContext* Parser);

		std::function<ds::ParsedClass* (ds::Token className, string derivedClass, string moduleName,
			ds::TokenStream& stream, string fileName)> CompileScript;
		void OnCompileFinished(UIParseData& Data);

	private:
		void RegisterChildren(kui::markup::UIElement* Element, ds::ParsedClass* Class, ds::ParseContext* Context);
		std::map<ds::ParsedClass*, ClassMapping> ClassMappings;
		std::vector<kui::markup::FileEntry> Files;
	};
}