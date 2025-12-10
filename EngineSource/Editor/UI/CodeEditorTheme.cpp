#include "CodeEditorTheme.h"
#include <Core/File/TextSerializer.h>
#include <Core/Log.h>

using namespace engine;

void engine::editor::CodeEditorTheme::ApplyToScript(ScriptEditorProvider* Provider) const
{
	ApplyToFile(Provider);

	Provider->FunctionColor = Function;
	Provider->VariableColor = Variable;
	Provider->TypeColor = Type;
}

void engine::editor::CodeEditorTheme::ApplyToFile(kui::FileEditorProvider* Provider) const
{
	Provider->TextColor = this->Text;
	Provider->KeywordColor = Keyword;
	Provider->StringColor = String;
	Provider->NumberColor = Number;
	Provider->BacketAreaColor = BracketArea;
	Provider->LineNumberColor = LineNumber;
}

void engine::editor::CodeEditorTheme::LoadFromFile(string ThemeName)
{
	*this = CodeEditorTheme();

	this->Name = ThemeName;

	try
	{
		SerializedValue ThemeData = TextSerializer::FromFile("Engine/Editor/Themes/Code/" + ThemeName + ".k2et");

		std::map<string, kui::Vec3f&> Colors =
		{
			{"text", this->Text},
			{"type", this->Type},
			{"function", this->Function},
			{"variable", this->Variable},
			{"string", this->String},
			{"number", this->Number},
			{"keyword", this->Keyword},
			{"bracketArea", this->BracketArea},
			{"lineNumber", this->LineNumber},
		};

		auto& ColorData = ThemeData.At("colors");

		this->IsLight = ThemeData.Contains("isLight") && ThemeData.At("isLight").GetBool();

		for (auto& [Name, Value] : Colors)
		{
			if (ColorData.Contains(Name))
			{
				Vector3 ValueVector = ColorData.At(Name).GetVector3();
				Value = kui::Vec3f(ValueVector.X, ValueVector.Y, ValueVector.Z);
			}
			else
			{
				Log::Note("Theme does not contain color - " + Name);
			}
		}
	}
	catch (SerializeReadException e)
	{
		Log::Warn(e.what());
	}
}
