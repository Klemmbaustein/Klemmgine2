#include "ParseUI.h"
#include <sstream>
#include <Markup/MarkupVerify.h>
#include <Markup/ParseError.h>
#include <Engine/Script/ScriptSubsystem.h>
#include <kui/UI/UIText.h>

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

	ParseResult Parsed = ParseFiles(Files, &Options);
	kui::markup::Verify(Parsed);

	ds::TokenStream DSharpTokens;

	for (auto& Element : Parsed.Elements)
	{
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
		if (this->CompileScript)
		{
			cls = CompileScript(ConvertToken(Element.FromToken), "game", DSharpTokens, Element.FilePath);
		}
		else
		{
			cls = Parser->addClass(ConvertToken(Element.FromToken), "game", DSharpTokens, Element.FilePath, {
				{ { ds::Token("engine::UIElement") } }
				});
		}

		RegisterChildren(&Element.Root, cls, Parser);
	}

	return UIParseData{
		.UIData = Parsed,
	};
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

		auto t = Context->programModules["engine"].moduleTypes["UIText"];

		Class->addMember(ConvertToken(i.ElementName), t,
			std::vector<ds::Token>({
			"this",
			".",
			"getChild",
			"<",
			"engine::" + i.TypeName.Text,
			">",
			"(",
			"\"" + i.ElementName.Text + "\"",
			")" }), true);
	}
}
