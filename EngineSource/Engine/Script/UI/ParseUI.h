#pragma once
#include <ds/parser/parser.hpp>
#include <Core/Types.h>
#include <Markup/MarkupParse.h>

namespace engine::script::ui
{
	struct UIParseData
	{
		kui::markup::ParseResult UIData;

		std::map<ds::TypeId, std::string> MarkupTypes;
	};

	class UIFileParser
	{
	public:
		UIFileParser();

		void AddFile(string Path);
		void AddString(string Path, const string& Content);

		UIParseData Parse(ds::ParseContext* Parser);

		std::function<ds::ParsedClass* (ds::Token className, std::string moduleName,
			ds::TokenStream& stream, std::string fileName)> CompileScript;

	private:
		void RegisterChildren(kui::markup::UIElement* Element, ds::ParsedClass* Class, ds::ParseContext* Context);

		std::vector<kui::markup::FileEntry> Files;
	};
}