#include "ScriptEditorPanel.h"
#include <Core/File/FileUtil.h>
#include <Editor/Server/EditorServerSubsystem.h>
#include <Editor/Settings/EditorSettings.h>
#include <Editor/UI/CodeEditorTheme.h>
#include <Editor/UI/EditorUI.h>
#include <Editor/UI/Elements/Toolbar.h>
#include <Engine/Engine.h>
#include <Engine/Script/ScriptSubsystem.h>
#include <filesystem>
#include <fstream>

using namespace kui;
using namespace engine::script;
using namespace engine::editor;

engine::editor::ScriptEditorPanel::ScriptEditorPanel()
	: EditorPanel("Scripts", "scripts")
{
	Toolbar* EditorToolbar = new Toolbar();
	EditorToolbar->AddButton("Save", EditorUI::Asset("Save.png"), [this]() {
		Save();
	});

	EditorToolbar->AddButton("Undo", EditorUI::Asset("Undo.png"), [this]() {
		GetSelectedTab()->Provider->Undo();
	});

	EditorToolbar->AddButton("Redo", EditorUI::Asset("Redo.png"), [this]() {
		GetSelectedTab()->Provider->Redo();
	});


	EditorToolbar->AddDropdown("Settings", EditorUI::Asset("Settings.png"),
		[this]() -> std::vector<DropdownMenu::Option> {
		auto TrimWhitespace = Settings::GetInstance()->Script.GetSetting("trimWhitespace", true).GetBool();
		return {
			DropdownMenu::Option{
				.Name = "Trim whitespace on save",
				.Icon = TrimWhitespace ? EditorUI::Asset("Dot.png") : "",
				.OnClicked = [TrimWhitespace]() {
					Settings::GetInstance()->Script.SetSetting("trimWhitespace", !TrimWhitespace);
				},
			},
		};
	});

	EditorToolbar->SetLeftPadding(0);

	SeparatorBackgrounds[0] = new UIBackground(true, 0, EditorUI::Theme.BackgroundHighlight,
		SizeVec(1_px, UISize::Parent(1)));
	SeparatorBackgrounds[1] = new UIBackground(true, 0, EditorUI::Theme.BackgroundHighlight,
		SizeVec(UISize::Parent(1), 1_px));

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

	EditorBox = new UIBox(true);

	CenterBox->AddChild(EditorBox
		->SetPadding(5_px));
	CenterBox->AddChild((SeparatorBackgrounds[1])
		->SetPadding(0, 0, 0, 1_px));

	StatusText = new UIText(12_px, EditorUI::Theme.Text, "Script status", EditorUI::EditorFont);

	CenterBox->AddChild(StatusText
		->SetPadding(2_px, 2_px, 5_px, 5_px));

	AddShortcut(Key::s, Key::LCTRL, [this] {
		Save();
		EditorUI::SetStatusMessage(str::Format("Saved script file '%s'", GetSelectedTab()->Provider->EditedFile.c_str()),
			EditorUI::StatusType::Info);
	}, ShortcutOptions::AllowInText);

	AddShortcut(Key::z, Key::LCTRL, [this] {
		GetSelectedTab()->Provider->Undo();
	}, ShortcutOptions::AllowInText);

	AddShortcut(Key::y, Key::LCTRL, [this] {
		GetSelectedTab()->Provider->Redo();
	}, ShortcutOptions::AllowInText);

	AddShortcut(Key::F12, {}, [this] {
		auto Hovered = GetSelectedTab()->Provider->GetSymbolAt(GetSelectedTab()->Editor->SelectionEnd);

		if (Hovered.GetDefinition)
		{
			auto Def = Hovered.GetDefinition();
			NavigateTo(Def.File, Def.Token.position);
			EditorUI::SetStatusMessage(str::Format("Navigated to definition of '%s'",
				Def.Token.string.c_str()), EditorUI::StatusType::Info);
		}
		else
		{
			EditorUI::SetStatusMessage("No definition available", EditorUI::StatusType::Warning);
		}
	}, ShortcutOptions::AllowInText);

	AddTab("Scripts/test.ds");
	AddTab("Scripts/test2.ds");

	this->ScriptEditorContext::Initialize();

	Settings::GetInstance()->Script.ListenToSetting(this, "miniMap", [this](SerializedValue val) {
		if (val.GetBool())
		{
			for (auto& i : this->Tabs)
			{
				i.MiniMap = new ScriptMiniMap(i.Editor, i.Provider);
				i.MiniMap->BackgroundColor = EditorUI::Theme.Background;
			}
		}
		else
		{
			for (auto& i : this->Tabs)
			{
				delete i.MiniMap;
				i.MiniMap = nullptr;
			}
		}
	});

	UpdateEditorTabs();
}

engine::editor::ScriptEditorPanel::~ScriptEditorPanel()
{
	Settings::GetInstance()->Script.RemoveListener(this);

	for (auto& i : Tabs)
	{
		if (i.MiniMap)
			delete i.MiniMap;
		delete i.Editor;
		delete i.Provider;
	}
}

void engine::editor::ScriptEditorPanel::UpdateTabSize(ScriptEditorTab* Tab)
{
	if (Tab->MiniMap)
		Tab->MiniMap->ReGenerate = true;
	Tab->Editor->UpdateElement();
}

ScriptEditorTab* engine::editor::ScriptEditorPanel::GetSelectedTab()
{
	if (SelectedTab >= Tabs.size())
	{
		return nullptr;
	}
	return &Tabs[SelectedTab];
}

void engine::editor::ScriptEditorPanel::UpdateEditorTabs()
{
	TabBox->DeleteChildren();

	auto* Selected = GetSelectedTab();

	for (size_t i = 0; i < Tabs.size(); i++)
	{
		auto& t = Tabs[i];
		t.Editor->IsVisible = false;
		Vec3f BgColor = Selected == &t ? EditorUI::Theme.HighlightDark : EditorUI::Theme.Background;
		Vec3f OutlineColor = Selected == &t ? EditorUI::Theme.Highlight1 : EditorUI::Theme.Background;

		TabBox->AddChild((new UIButton(true, 0, BgColor, [this, i]() {
			OpenTab(i);
		}))
			->SetBorder(1_px, OutlineColor)
			->SetCorner(EditorUI::Theme.CornerSize)
			->SetVerticalAlign(UIBox::Align::Centered)
			->SetMinWidth(UISize::Parent(1))
			->SetPadding(5_px, 0, 5_px, 5_px)
			->AddChild((new UIText(12_px, EditorUI::Theme.Text, file::FileName(t.Provider->EditedFile), EditorUI::EditorFont))
				->SetTextWidthOverride(152_px)
				->SetPadding(4_px))
			->AddChild((new UIButton(true, 0, EditorUI::Theme.Text, nullptr))
				->SetUseImage(true, EditorUI::Asset("X.png"))
				->SetMinSize(12_px)
				->SetPadding(4_px)));
	}
}

void engine::editor::ScriptEditorPanel::Save()
{
	auto Tab = GetSelectedTab();

	if (Tab)
	{
		std::filesystem::create_directories("Scripts/");

		std::ofstream out = std::ofstream(Tab->Provider->EditedFile);

		if (Settings::GetInstance()->Script.GetSetting("trimWhitespace", true).GetBool())
		{
			Tab->Provider->TrimWhitespace(Tab->Editor->SelectionEnd.Line);
		}

		out << Tab->Provider->GetContent();
		out.close();

		Engine::Instance->GetSubsystem<ScriptSubsystem>()->Reload();
		EditorUI::SetStatusMessage("Compiled scripts", EditorUI::StatusType::Info);
	}
	else
	{
		EditorUI::SetStatusMessage("Could not save scripts because no file is active.", EditorUI::StatusType::Info);
	}
}

void engine::editor::ScriptEditorPanel::NavigateTo(std::string File, ds::TokenPos at)
{
	for (size_t i = 0; i < Tabs.size(); i++)
	{
		if (Tabs[i].Provider->EditedFile != File)
		{
			continue;
		}

		if (SelectedTab == i)
		{
			Tabs[i].Provider->NavigateTo(EditorPosition(at.startPos, at.line),
				EditorPosition(at.endPos, at.line));
		}
		else
		{
			OpenTab(i);
			Tabs[i].Provider->NavigateTo(EditorPosition(at.startPos, at.line),
				EditorPosition(at.endPos, at.line));
		}
	}
}

void engine::editor::ScriptEditorPanel::OnResized()
{
	this->EditorBox->SetMinHeight(this->Size.Y - (78_px).GetScreen().Y);
	this->EditorBox->SetMinWidth(this->Size.X - (200_px).GetScreen().X);
	this->EditorBox->GetAbsoluteParent()->UpdateElement();
	UpdateTabSize(GetSelectedTab());
}

void engine::editor::ScriptEditorPanel::Update()
{
	auto Tab = GetSelectedTab();
	if (Tab)
	{
		Tab->Editor->IsVisible = this->Visible;
		Tab->Editor->SetMinWidth(EditorBox->GetUsedSize().X);
		Tab->Editor->SetMinHeight(EditorBox->GetUsedSize().Y);
		Tab->Editor->SetPosition(EditorBox->GetScreenPosition());

		if (this->Visible)
		{
			if (Tab->MiniMap)
			{
				Tab->MiniMap->Update();
			}
			this->StatusText->SetText(str::Format("%s | Errors: %i",
				Tab->Provider->EditedFile.c_str(), this->Errors[Tab->Provider->EditedFile].size()));

			auto sys = Engine::Instance->GetSubsystem<EditorServerSubsystem>();
			if (sys && !Tab->Provider->Connection)
			{
				Tab->Provider->Connection = sys->Connection;
				Tab->Provider->LoadRemoteFile();
			}
		}
	}
}

void engine::editor::ScriptEditorPanel::OnThemeChanged()
{
	StatusText->SetColor(EditorUI::Theme.Text);
	for (auto& i : SeparatorBackgrounds)
	{
		i->SetColor(EditorUI::Theme.BackgroundHighlight);
	}

	for (auto& Tab : Tabs)
	{
		if (Tab.MiniMap)
			Tab.MiniMap->BackgroundColor = EditorUI::Theme.Background;
		EditorUI::Theme.CodeTheme.ApplyToScript(Tab.Provider);
		Tab.Provider->ParentEditor->SelectionColor = EditorUI::Theme.SelectedText;
		Tab.Provider->ParentEditor->CursorColor = EditorUI::Theme.Text;
		Tab.Provider->RefreshAll();
		UpdateEditorTabs();
	}
}

void engine::editor::ScriptEditorPanel::AddTab(std::string File)
{
	auto& NewTab = this->Tabs.emplace_back();
	NewTab.Provider = new ScriptEditorProvider(File, this);

	NewTab.Provider->Keywords = {
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

	EditorUI::Theme.CodeTheme.ApplyToScript(NewTab.Provider);
	if (this->Loaded)
	{
		NewTab.Provider->ScanFile();
	}
	NewTab.Editor = new UITextEditor(NewTab.Provider, EditorUI::MonospaceFont);

	if (Settings::GetInstance()->Script.GetSetting("miniMap", true).GetBool())
	{
		NewTab.MiniMap = new ScriptMiniMap(NewTab.Editor, NewTab.Provider);
		NewTab.MiniMap->BackgroundColor = EditorUI::Theme.Background;
	}

	this->AdditionalBoxes.push_back(NewTab.Editor);
}

void engine::editor::ScriptEditorPanel::OpenTab(size_t Tab)
{
	if (SelectedTab == Tab)
	{
		return;
	}

	if (auto Previous = GetSelectedTab())
	{
		Previous->Provider->ClearHovered();
	}

	SelectedTab = Tab;
	GetSelectedTab()->Editor->UpdateHighlights = true;
	UpdateEditorTabs();
	OnResized();
}
