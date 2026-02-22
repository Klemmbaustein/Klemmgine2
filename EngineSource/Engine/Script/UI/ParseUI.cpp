#include "ParseUI.h"
#include <sstream>
#include <Markup/MarkupVerify.h>
#include <Markup/ParseError.h>
#include <Engine/Script/ScriptSubsystem.h>
#include <ds/service/languageService.hpp>

using namespace engine;
using namespace engine::script::ui;
using namespace kui::markup;

engine::script::ui::UIFileParser::UIFileParser()
{
}

void engine::script::ui::UIFileParser::AddFile(string Path)
{
	std::ifstream FileContent = std::ifstream(Path);

	std::stringstream FileContentStream;
	FileContentStream << FileContent.rdbuf();
	FileContent.close();

	Files.push_back(FileEntry{
		.Content = FileContentStream.str(),
		.Name = Path,
		.Path = Path,
		});
}

void engine::script::ui::UIFileParser::AddString(string Path, const string& Content)
{
	Files.push_back(FileEntry{
		.Content = Content,
		.Name = Path,
		.Path = Path,
		});
}

static ds::Token ConvertToken(kui::stringParse::StringToken t)
{
	return ds::Token(t, ds::TokenPos(t.BeginChar, t.EndChar, t.Line));
}

UIParseData engine::script::ui::UIFileParser::Parse(ds::ParseContext* Parser)
{
	auto Tokenize = [Parser](const std::string& String, const std::string& File)
		-> std::vector<kui::stringParse::Line> {
		std::vector<kui::stringParse::Line> Result;

		ds::TokenStream Stream;
		Stream.fromString(String, File, &Parser->errors);

		while (true)
		{
			auto next = Stream.next(&Parser->errors);

			if (next.empty())
			{
				break;
			}

			auto& NewLines = Result.emplace_back();

			for (auto& i : *next.lineTokens)
			{
				NewLines.Strings.push_back(kui::stringParse::StringToken(i.string,
					i.position.startPos, i.position.endPos, i.position.line));
			}
		}

		return Result;
	};

	ParseOptions Options = {
		.CustomFields = {"script"},
		.Tokenize = Tokenize,
	};

	kui::parseError::ErrorCallback = [this, Parser](std::string Message,
		std::string File, size_t Line, size_t Begin, size_t End) {
		Parser->errors.currentFile = File;
		Parser->errors.error(ds::ErrorCode(5000), ds::Token("", ds::TokenPos(Begin, End, Line)), Message);
	};

	UIParseData Result;
	Result.UIData = ParseFiles(Files, &Options);
	kui::markup::Verify(Result.UIData);

	for (auto& Element : Result.UIData.Elements)
	{
		ds::TokenStream DSharpTokens;
		for (auto& Segments : Element.CustomSegments)
		{
			for (auto& SegmentLine : Segments.second.Lines)
			{
				DSharpTokens.addLine();
				for (auto& SegmentToken : SegmentLine.Strings)
				{
					DSharpTokens.addToken(ConvertToken(SegmentToken));
				}
			}
		}

		ds::ParsedClass* cls = nullptr;

		if (Element.Derived.Text.empty())
		{
			Element.Derived.Text = "UIScriptElement";
		}

		auto File = Element.FilePath + "/" + Element.FromToken.Text;

		if (Parser->service)
		{
			Parser->service->files.erase(Element.FilePath);
		}

		auto DerivedToken = ds::Token("engine::ui::" + Element.Derived.Text,
			ds::TokenPos(Element.Derived.BeginChar, Element.Derived.EndChar, Element.Derived.Line));

		if (this->CompileScript)
		{
			cls = CompileScript(ConvertToken(Element.FromToken), DerivedToken, "", DSharpTokens, File);
		}
		else
		{
			cls = Parser->addClass(ConvertToken(Element.FromToken), "", DSharpTokens, File,
				{ { { DerivedToken }} });
		}

		if (cls)
		{
			cls->registerType(Parser, cls->definitionFile);
			cls->definitionFile->displayName = Element.FilePath;
		}
		ClassMappings.insert({
			cls,
			ClassMapping(Element)
			});
	}

	for (auto& i : ClassMappings)
	{
		if (i.first)
		{
			RegisterChildren(&i.second.Element.Root, i.first, Parser);
		}
	}

	return Result;
}

void engine::script::ui::UIFileParser::OnCompileFinished(UIParseData& Data)
{
	Data.ClassIdMappings.clear();
	for (auto& i : this->ClassMappings)
	{
		Data.ClassIdMappings[i.first->thisType->id] = i.second.Element.FromToken.Text;
	}
}

void engine::script::ui::UIFileParser::RegisterChildren(UIElement* Element,
	ds::ParsedClass* Class, ds::ParseContext* Context)
{
	for (auto& i : Element->Children)
	{
		RegisterChildren(&i, Class, Context);
		if (i.ElementName.Empty())
		{
			continue;
		}

		auto t = Context->programModules["engine::ui"].moduleTypes[i.TypeName.Text];
		string TypeName = "";

		if (t)
		{
			TypeName = "engine::ui::" + i.TypeName.Text;
		}
		else
		{
			for (auto& [MappedClass, Name] : this->ClassMappings)
			{
				if (Name.Element.FromToken == i.TypeName.Text)
				{
					t = MappedClass->thisType;
					TypeName = MappedClass->name.string;
				}
			}

			if (!t)
			{
				continue;
			}
		}

		Class->addMember(ConvertToken(i.ElementName), t,
			std::vector<ds::Token>({
			"this",
			".",
			"getChild",
			"<",
			TypeName,
			">",
			"(",
			"\"" + i.ElementName.Text + "\"",
			")" }), true);
	}
}
