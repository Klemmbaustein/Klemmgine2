#include "TextEditorPanel.h"
#include <Core/Log.h>
#include <Editor/UI/EditorUI.h>
#include <Editor/UI/Elements/Toolbar.h>
#include <language.hpp>
#include <modules/standardLibrary.hpp>
#include <Engine/Engine.h>
#include <Engine/Script/ScriptSubsystem.h>

using namespace kui;

engine::editor::TextEditorPanel::TextEditorPanel(AssetRef Asset)
	: AssetEditor("Raw view: %s", Asset)
{
	Provider = new FileEditorProvider(Asset.FilePath);

	Provider->Keywords = {
	"int",
	"float",
	"bool",
	"obj",
	"array",
	"vec3",
	"vec2",
	"obj",
	"str",
	"byte",
	};

	Toolbar* EditorToolbar = new Toolbar();
	EditorToolbar->AddButton("Save", EditorUI::Asset("Save.png"), [this]() {
		Save();
	});
	EditorToolbar->AddButton("Run", EditorUI::Asset("Run.png"), [this]() {
		RunScript();
	});
	this->Background->SetHorizontal(false);

	this->Background->AddChild(EditorToolbar);

	Editor = new UITextEditor(Provider, EditorUI::MonospaceFont);
	//Editor->EditorScrollBox->ScrollBarWidth = 20;

	this->Background->AddChild(Editor
		->SetPadding(5_px));
	this->Background->AddChild((new UIBackground(true, 0, EditorUI::Theme.BackgroundHighlight, SizeVec(UISize::Parent(1), 1_px)))
		->SetPadding(1_px));

	this->Background->AddChild((new UIText(12_px, EditorUI::Theme.Text, Asset.FilePath, EditorUI::EditorFont))
		->SetPadding(2_px, 2_px, 5_px, 5_px));
}

void engine::editor::TextEditorPanel::OnResized()
{
	this->Editor->SetMinHeight(this->Size.Y - (78_px).GetScreen().Y);
	this->Editor->SetMinWidth(UISize::Parent(1));
}

void engine::editor::TextEditorPanel::Save()
{
	std::ofstream out = std::ofstream(this->EditedAsset.FilePath);

	out << this->Provider->GetContent();
}

void engine::editor::TextEditorPanel::RunScript()
{
	using namespace lang;

	auto Language = Engine::GetSubsystem<script::ScriptSubsystem>()->ScriptLanguage;

	ParseContext* Compiler = Language->createCompiler();

	Compiler->errors.writeError = [](std::string str)
	{
		Log::Info(str);
	};

	Compiler->addString(this->Provider->GetContent(), this->EditedAsset.FilePath);

	BytecodeStream Code = Compiler->compile();

	InterpretContext* Interpreter = Language->createInterpreter();

	Interpreter->loadBytecode(&Code);
	Interpreter->run();
}