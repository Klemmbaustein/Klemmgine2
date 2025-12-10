#include "ScriptEditorPanel.h"
#include <Editor/UI/Elements/Toolbar.h>
#include <Editor/UI/CodeEditorTheme.h>
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
		"not",
		"or",
		"and",
		"const",
		"fn",
		"enum",
		"async",
		"await"
	};

	EditorUI::Theme.CodeTheme.ApplyToScript(Provider);
	Provider->ScanFile();

	Toolbar* EditorToolbar = new Toolbar();
	EditorToolbar->AddButton("Save", EditorUI::Asset("Save.png"), [this]() {
		Save();
	});

	EditorToolbar->AddButton("Undo", EditorUI::Asset("Undo.png"), [this]() {
		Provider->Undo();
	});

	EditorToolbar->AddButton("Redo", EditorUI::Asset("Redo.png"), [this]() {
		Provider->Redo();
	});

	EditorToolbar->SetLeftPadding(0);

	SeparatorBackgrounds[0] = new UIBackground(true, 0, EditorUI::Theme.BackgroundHighlight, SizeVec(1_px, UISize::Parent(1)));
	SeparatorBackgrounds[1] = new UIBackground(true, 0, EditorUI::Theme.BackgroundHighlight, SizeVec(UISize::Parent(1), 1_px));

	Background->SetHorizontal(true);
	CenterBox = new UIBox(false);
	TabBox = new UIScrollBox(false, 0, true);
	TabBox->SetMinWidth(189_px);
	TabBox->SetMinHeight(UISize::Parent(1));
	this->Background->AddChild(TabBox);
	this->Background->AddChild(
		SeparatorBackgrounds[0]
		->SetPadding(1_px, 1_px, 0, 0));
	this->Background->AddChild(CenterBox);

	CenterBox->AddChild(EditorToolbar);

	Editor = new UITextEditor(Provider, EditorUI::MonospaceFont);


	CenterBox->AddChild(Editor
		->SetPadding(5_px));
	CenterBox->AddChild((SeparatorBackgrounds[1])
		->SetPadding(0, 0, 0, 1_px));

	StatusText = new UIText(12_px, EditorUI::Theme.Text, "Script status", EditorUI::EditorFont);

	CenterBox->AddChild(StatusText
		->SetPadding(2_px, 2_px, 5_px, 5_px));

	MiniMap = new ScriptMiniMap(Editor, Provider);

	MiniMap->BackgroundColor = EditorUI::Theme.Background;

	AddShortcut(Key::s, Key::LCTRL, [this] {
		Save();
		EditorUI::SetStatusMessage(str::Format("Saved file %s", Provider->ScriptFile.c_str()), EditorUI::StatusType::Info);
	}, ShortcutOptions::AllowInText);

	AddShortcut(Key::z, Key::LCTRL, [this] {
		Provider->Undo();
	}, ShortcutOptions::AllowInText);

	AddShortcut(Key::y, Key::LCTRL, [this] {
		Provider->Redo();
	}, ShortcutOptions::AllowInText);
	UpdateTabs();
}

void engine::editor::ScriptEditorPanel::UpdateTabs()
{
	TabBox->DeleteChildren();

	TabBox->AddChild((new UIButton(true, 0, EditorUI::Theme.HighlightDark, nullptr))
		->SetBorder(1_px, EditorUI::Theme.Highlight1)
		->SetCorner(EditorUI::Theme.CornerSize)
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
	EditorUI::SetStatusMessage("Compiled scripts", EditorUI::StatusType::Info);
}

void engine::editor::ScriptEditorPanel::OnResized()
{
	this->Editor->SetMinHeight(this->Size.Y - (78_px).GetScreen().Y);
	this->Editor->SetMinWidth(this->Size.X - (200_px).GetScreen().X);
	this->Editor->UpdateElement();
	this->MiniMap->ReGenerate = true;
}

void engine::editor::ScriptEditorPanel::Update()
{
	this->MiniMap->Update();
	this->StatusText->SetText(str::Format("%s | Errors: %i", this->Provider->ScriptFile.c_str(), this->Provider->Errors.size()));

	auto sys = Engine::Instance->GetSubsystem<EditorServerSubsystem>();
	if (sys && !Provider->Connection)
	{
		Provider->Connection = sys->Connection;
		Provider->LoadRemoteFile();
	}
}

void engine::editor::ScriptEditorPanel::OnThemeChanged()
{
	MiniMap->BackgroundColor = EditorUI::Theme.Background;
	StatusText->SetColor(EditorUI::Theme.Text);

	for (auto& i : SeparatorBackgrounds)
	{
		i->SetColor(EditorUI::Theme.BackgroundHighlight);
	}

	EditorUI::Theme.CodeTheme.ApplyToScript(Provider);
	Provider->RefreshAll();
	UpdateTabs();
}
