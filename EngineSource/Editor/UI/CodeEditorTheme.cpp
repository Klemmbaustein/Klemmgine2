#include "CodeEditorTheme.h"

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
}

void engine::editor::CodeEditorTheme::LoadFromFile(string ThemeName)
{
}
