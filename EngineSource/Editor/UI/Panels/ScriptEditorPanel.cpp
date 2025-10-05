#include "ScriptEditorPanel.h"
#include <Editor/UI/Elements/Toolbar.h>
#include <Engine/Engine.h>
#include <Engine/Script/ScriptSubsystem.h>
#include <Editor/UI/EditorUI.h>
#include <Editor/Server/EditorServerSubsystem.h>
#include <fstream>

using namespace kui;
using namespace engine::script;

engine::editor::ScriptEditorPanel::ScriptEditorPanel()
	: EditorPanel("Scripts", "scripts")
{
	Provider = new ScriptEditorProvider("Scripts/test.ds");

	Provider->Keywords = {
		"int",
		"float",
		"bool",
		"string",
		"if",
		"else",
		"for",
		"while",
		"null",
		"true",
		"false",
		"new",
		"break",
		"continue",
		"using",
		"module",
		"_",
		"virtual",
		"override",
		"var",
		"class",
		"struct",
		"return",
		"in",
		"const",
		"fn",
		"enum",
	};

	Toolbar* EditorToolbar = new Toolbar();
	EditorToolbar->AddButton("Save", EditorUI::Asset("Save.png"), [this]() {
		Save();
	});

	EditorToolbar->SetLeftPadding(0);

	Background->SetHorizontal(true);
	CenterBox = new UIBox(false);
	TabBox = new UIScrollBox(false, 0, true);
	TabBox->SetMinWidth(189_px);
	TabBox->SetMinHeight(UISize::Parent(1));
	this->Background->AddChild(TabBox);
	this->Background->AddChild(
		(new UIBackground(true, 0, EditorUI::Theme.BackgroundHighlight, SizeVec(1_px, UISize::Parent(1)))));
	this->Background->AddChild(CenterBox);

	CenterBox->AddChild(EditorToolbar);

	Editor = new UITextEditor(Provider, EditorUI::MonospaceFont);
	//Editor->EditorScrollBox->ScrollBarWidth = 20;

	CenterBox->AddChild(Editor
		->SetPadding(5_px));
	CenterBox->AddChild((new UIBackground(true, 0, EditorUI::Theme.BackgroundHighlight, SizeVec(UISize::Parent(1), 1_px)))
		->SetPadding(1_px));

	CenterBox->AddChild((new UIText(12_px, EditorUI::Theme.Text, "Script status", EditorUI::EditorFont))
		->SetPadding(2_px, 2_px, 5_px, 5_px));

	UpdateTabs();
}

void engine::editor::ScriptEditorPanel::UpdateTabs()
{
	TabBox->DeleteChildren();

	TabBox->AddChild((new UIButton(true, 0, EditorUI::Theme.HighlightDark, nullptr))
		->SetBorder(1_px, EditorUI::Theme.Highlight1)
		->SetCorner(5_px)
		->SetVerticalAlign(UIBox::Align::Centered)
		->SetMinWidth(UISize::Parent(1))
		->SetPadding(5_px)
		->AddChild((new UIText(12_px, EditorUI::Theme.Text, "test.ds", EditorUI::EditorFont))
			->SetTextWidthOverride(152_px)
			->SetPadding(4_px))
		->AddChild((new UIButton(true, 0, EditorUI::Theme.Text, nullptr))
			->SetUseImage(true, EditorUI::Asset("X.png"))
			->SetMinSize(12_px)
			->SetPadding(4_px)));
}

void engine::editor::ScriptEditorPanel::Save()
{
	std::ofstream out = std::ofstream(Provider->ScriptFile);

	out << this->Provider->GetContent();
	out.close();

	Engine::Instance->GetSubsystem<ScriptSubsystem>()->Reload();
}

void engine::editor::ScriptEditorPanel::OnResized()
{
	this->Editor->SetMinHeight(this->Size.Y - (78_px).GetScreen().Y);
	this->Editor->SetMinWidth(this->Size.X - (200_px).GetScreen().X);
}

void engine::editor::ScriptEditorPanel::Update()
{
	auto sys = Engine::Instance->GetSubsystem<EditorServerSubsystem>();
	if (sys && !Provider->Connection)
	{
		Provider->Connection = sys->Connection;
		Provider->LoadRemoteFile();
	}
}
