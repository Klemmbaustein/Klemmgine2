#include "ScriptEditorUI.h"
#include <Core/File/FileUtil.h>
#include <Editor/UI/EditorUI.h>
#include <Core/File/JsonSerializer.h>
#include <filesystem>
#include <Editor/UI/Panels/ClassBrowser.h>
#include <Engine/Engine.h>
#include <Engine/Script/ScriptSubsystem.h>
#include <fstream>
#include <Editor/Settings/EditorSettings.h>
#include <Editor/UI/Windows/SettingsWindow.h>
#include <Editor/UI/CodeEditorTheme.h>
#include <Engine/MainThread.h>

using namespace kui;
using namespace engine;
using namespace engine::editor;
using namespace engine::script;

engine::editor::ScriptEditorUI::ScriptEditorUI(kui::UIBox* Background, bool IsFloating,
	kui::Font* TextFont, kui::Font* ScriptFont, thread::ThreadMessagesRef Queue)
{
	this->Queue = Queue;
	this->ScriptFont = ScriptFont;
	this->TextFont = TextFont;
	this->Background = Background;
	this->IsFloating = IsFloating;

	ScriptToolbar = new Toolbar(!IsFloating, IsFloating ? EditorUI::Theme.DarkBackground : -1);
	ScriptToolbar->AddButton("Save", EditorUI::Asset("Save.png"), [this]() {
		Save();
	});

	ScriptToolbar->AddButton("Undo", EditorUI::Asset("Undo.png"), [this]() {
		GetSelectedTab()->Provider->Undo();
	});

	ScriptToolbar->AddButton("Redo", EditorUI::Asset("Redo.png"), [this]() {
		GetSelectedTab()->Provider->Redo();
	});

	ScriptToolbar->AddDropdown("Settings", EditorUI::Asset("Settings.png"),
		[this]() -> std::vector<DropdownMenu::Option> {
		auto TrimWhitespace = Settings::GetInstance()->Script.GetSetting("trimWhitespace", true).GetBool();
		return {
			DropdownMenu::Option{
				.Name = "Trim whitespace on save",
				.Icon = TrimWhitespace ? EditorUI::Asset("Dot.png") : "",
				.OnClicked = [TrimWhitespace]() {
					Settings::GetInstance()->Script.SetSetting("trimWhitespace", !TrimWhitespace);
				},
				.Separator = true,
			},
			DropdownMenu::Option{
				.Name = "Open all script settings",
				.Icon = EditorUI::Asset("Open.png"),
				.OnClicked = []() {
					new SettingsWindow("Script Editor");
				},
			},
		};
	});

	ScriptToolbar->SetLeftPadding(0);

	SeparatorBackgrounds[0] = new UIBackground(true, 0, EditorUI::Theme.BackgroundHighlight,
		SizeVec(1_px, UISize::Parent(1)));
	SeparatorBackgrounds[1] = new UIBackground(true, 0, EditorUI::Theme.BackgroundHighlight,
		SizeVec(UISize::Parent(1), 1_px));

	Background->SetHorizontal(true);
	CenterBox = new UIBox(false);
	VerticalTabBox = new UIScrollBox(false, 0, true);
	VerticalTabBox->SetMinWidth(189_px);
	VerticalTabBox->SetMinHeight(UISize::Parent(1));
	HorizontalTabBox = new UIScrollBox(true, 0, false);

	HorizontalTabBox->SetMinWidth(UISize::Parent(1));
	HorizontalTabBox->SetMinHeight(20_px);

	if (IsFloating)
	{
		VerticalTabBackground = new UIBackground(true, 0, EditorUI::Theme.DarkBackground);

		Background->AddChild(VerticalTabBackground
			->SetMinHeight(UISize::Parent(1))
			->AddChild(VerticalTabBox));
	}
	else
	{
		Background->AddChild(VerticalTabBox);
	}

	Background->AddChild(
		SeparatorBackgrounds[0]
		->SetPadding(1_px, 1_px, 0, 0));
	Background->AddChild(CenterBox);

	CenterBox->AddChild(ScriptToolbar);
	CenterBox->AddChild(HorizontalTabBox);

	EditorBox = new UIBox(true);

	CenterBox->AddChild(EditorBox
		->SetPadding(5_px));
	CenterBox->AddChild((SeparatorBackgrounds[1])
		->SetPadding(0, 0, 0, 1_px));

	StatusText = new UIText(12_px, EditorUI::Theme.Text, "Script status", TextFont);

	CenterBox->AddChild(StatusText
		->SetPadding(2_px, 2_px, 5_px, 5_px));

	AddShortcut(Key::s, ShortcutModifiers{ .Ctrl = true }, [this] {
		Save();
	}, ShortcutOptions::AllowInText);

	AddShortcut(Key::z, ShortcutModifiers{ .Ctrl = true }, [this] {
		GetSelectedTab()->Provider->Undo();
	}, ShortcutOptions::AllowInText);

	AddShortcut(Key::y, ShortcutModifiers{ .Ctrl = true }, [this] {
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

	auto LastOpened = GetLastOpenedFiles();
	for (auto& i : LastOpened)
	{
		AddTab(i);
	}

	this->ScriptEditorContext::Initialize();

	InitializeSettings();

	UpdateEditorTabs();
}

engine::editor::ScriptEditorUI::~ScriptEditorUI()
{
	SaveLastOpenedFiles();
	Settings::GetInstance()->Script.RemoveListener(this);

	Quit = true;

	for (auto& i : Tabs)
	{
		if (i.MiniMap)
			delete i.MiniMap;
		delete i.Editor;
		delete i.Provider;
	}
}

void engine::editor::ScriptEditorUI::Update()
{
	auto Selected = GetSelectedTab();
	for (auto& Tab : Tabs)
	{
		Tab.Editor->SetMinWidth(EditorBox->GetUsedSize().X);
		Tab.Editor->SetMinHeight(EditorBox->GetUsedSize().Y);
		Tab.Editor->SetMaxWidth(EditorBox->GetUsedSize().X);
		Tab.Editor->SetMaxHeight(EditorBox->GetUsedSize().Y);
		Tab.Editor->SetPosition(EditorBox->GetScreenPosition());
		if (&Tab != Selected || !this->IsVisible)
		{
			Tab.Provider->ClearHovered();
		}
	}

	if (Selected)
	{
		Selected->Editor->IsVisible = this->IsVisible;
		if (this->IsVisible)
		{
			if (Selected->MiniMap)
			{
				Selected->MiniMap->Update();
			}
			this->StatusText->SetText(str::Format("%s | Errors: %i",
				Selected->Provider->EditedFile.c_str(), this->Errors[Selected->Provider->EditedFile].size()));
		}
	}
}

void engine::editor::ScriptEditorUI::UpdateTabSize(ScriptEditorTab* Tab)
{
	if (Tab)
	{
		if (Tab->MiniMap)
			Tab->MiniMap->ReGenerate = true;
		Tab->Editor->UpdateElement();
	}
}

ScriptEditorTab* engine::editor::ScriptEditorUI::GetSelectedTab()
{
	if (SelectedTab >= Tabs.size())
	{
		return nullptr;
	}
	return &Tabs[SelectedTab];
}

void engine::editor::ScriptEditorUI::CloseTab(size_t Index)
{
	if (SelectedTab >= Index && SelectedTab > 0)
	{
		SelectedTab--;
	}
	if (Tabs[Index].MiniMap)
		delete Tabs[Index].MiniMap;
	delete Tabs[Index].Editor;
	delete Tabs[Index].Provider;
	Tabs.erase(Tabs.begin() + Index);
	UpdateEditorTabs();
}

void engine::editor::ScriptEditorUI::InitializeSettings()
{
	Settings::GetInstance()->Script.ListenToSetting(this, "miniMap", [this](SerializedValue val) {
		if (val.GetBool())
		{
			for (auto& i : this->Tabs)
			{
				i.MiniMap = new ScriptMiniMap(i.Editor, i.Provider, Queue);
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
	}, Queue);

	Settings::GetInstance()->Script.ListenToSetting(this, "useVerticalTabs", [this](SerializedValue val) {
		UseVerticalTabs = val.GetBool();
		UpdateEditorTabs();
		OnResized(Size);
	}, Queue);

	UseVerticalTabs = Settings::GetInstance()->Script.GetSetting("useVerticalTabs", true).GetBool();
}

void engine::editor::ScriptEditorUI::AddTab(std::string File)
{
	auto& NewTab = this->Tabs.emplace_back();
	NewTab.Provider = new ScriptEditorProvider(File, this, Queue);

	NewTab.Provider->Keywords = {
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
		"await",
		"as",
		"is",
		"interface"
	};

	if (file::Extension(File) == "kui")
	{
		NewTab.Provider->Keywords.insert("element");
		NewTab.Provider->Keywords.insert("child");
		NewTab.Provider->Keywords.insert("global");
		NewTab.Provider->Keywords.insert("horizontal");
		NewTab.Provider->Keywords.insert("vertical");
		NewTab.Provider->Keywords.insert("centered");
		NewTab.Provider->Keywords.insert("reverse");
		NewTab.Provider->Keywords.insert("default");
		NewTab.Provider->Keywords.insert("script");
	}

	EditorUI::Theme.CodeTheme.ApplyToScript(NewTab.Provider);
	if (this->Loaded)
	{
		NewTab.Provider->ScanFile();
	}
	NewTab.Editor = new UITextEditor(NewTab.Provider, ScriptFont);

	if (Settings::GetInstance()->Script.GetSetting("miniMap", true).GetBool())
	{
		NewTab.MiniMap = new ScriptMiniMap(NewTab.Editor, NewTab.Provider, Queue);
		NewTab.MiniMap->BackgroundColor = EditorUI::Theme.Background;
	}

	UpdateEditorTabs();
}

void engine::editor::ScriptEditorUI::OpenTab(size_t Tab)
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
	OnResized(Size);
}

void engine::editor::ScriptEditorUI::UpdateEditorTabs()
{
	VerticalTabBox->DeleteChildren();
	HorizontalTabBox->DeleteChildren();
	VerticalTabBox->IsCollapsed = !UseVerticalTabs;
	HorizontalTabBox->IsCollapsed = UseVerticalTabs;
	auto TabBox = UseVerticalTabs ? VerticalTabBox : HorizontalTabBox;

	auto* Selected = GetSelectedTab();

	for (size_t i = 0; i < Tabs.size(); i++)
	{
		auto& t = Tabs[i];
		t.Editor->IsVisible = false;

		auto Background = IsFloating && UseVerticalTabs ? EditorUI::Theme.DarkBackground : EditorUI::Theme.Background;

		Vec3f BgColor = Selected == &t ? EditorUI::Theme.HighlightDark : Background;
		Vec3f OutlineColor = Selected == &t ? EditorUI::Theme.Highlight1 : Background;

		string Name = file::FileName(t.Provider->EditedFile);

		t.TabName = new UIText(12_px, EditorUI::Theme.Text, Name, TextFont);

		if (UseVerticalTabs)
		{
			t.TabName->SetTextWidthOverride(152_px);
		}

		auto btn = new UIButton(true, 0, BgColor, [this, i]() {
			OpenTab(i);
		});

		if (Selected != &t)
		{
			btn->SetHoveredColor(EditorUI::Theme.HighlightDark);
			btn->SetPressedColor(EditorUI::Theme.HighlightDark);
		}

		TabBox->AddChild(btn
			->SetBorder(1_px, OutlineColor)
			->SetCorner(EditorUI::Theme.CornerSize)
			->SetVerticalAlign(UIBox::Align::Centered)
			->SetPadding(5_px, 0, 5_px, 5_px)
			->AddChild(t.TabName
				->SetPadding(4_px))
			->AddChild((new UIButton(true, 0, EditorUI::Theme.Text, [this, i]() {
			CloseTab(i);
		}))
				->SetUseImage(true, EditorUI::Asset("X.png"))
				->SetMinSize(12_px)
				->SetPadding(4_px)));
	}
}

void engine::editor::ScriptEditorUI::Save()
{
	auto Tab = GetSelectedTab();

	if (Tab)
	{
		std::ofstream out = std::ofstream(Tab->Provider->EditedFile);

		if (Settings::GetInstance()->Script.GetSetting("trimWhitespace", true).GetBool())
		{
			Tab->Provider->TrimWhitespace(Tab->Editor->SelectionEnd.Line);
		}

		out << Tab->Provider->GetContent();
		out.close();
		Tab->TabName->SetText(file::FileName(Tab->Provider->EditedFile));
		Tab->IsSaved = true;

		auto fn = [] {
			bool CompileResult = Engine::Instance->GetSubsystem<ScriptSubsystem>()->Reload();
			if (CompileResult)
			{
				EditorUI::SetStatusMessage("Compiled scripts", EditorUI::StatusType::Info);
			}
			else
			{
				EditorUI::SetStatusMessage("Failed to compile scripts", EditorUI::StatusType::Error);
			}

			EditorUI::Instance->ForEachPanel<ClassBrowser>([](ClassBrowser* Browser) {
				Browser->UpdateItems();
			});
		};

		if (!thread::IsMainThread)
		{
			thread::ExecuteOnMainThread(fn);
		}
		else
		{
			fn();
		}
	}
	else
	{
		EditorUI::SetStatusMessage("Could not save scripts because no file is active.", EditorUI::StatusType::Warning);
	}
}

std::set<string> engine::editor::ScriptEditorUI::GetLastOpenedFiles()
{
	if (!std::filesystem::exists(TABS_OPENED_FILE))
	{
		return std::set<string>();
	}

	try
	{
		auto Files = JsonSerializer::FromFile(TABS_OPENED_FILE);

		std::set<string> FoundFiles;

		for (auto& i : Files.GetArray())
		{
			FoundFiles.insert(i.GetString());
		}
		return FoundFiles;
	}
	catch (SerializeException& e)
	{
		Log::Warn(str::Format("Failed to load last saved tabs from the file %s: %s", TABS_OPENED_FILE, e.what()));
		return {};
	}
}

void engine::editor::ScriptEditorUI::SaveLastOpenedFiles()
{
	std::vector<SerializedValue> Result;

	for (auto& i : Tabs)
	{
		Result.push_back(SerializedValue(i.Provider->EditedFile));
	}

	JsonSerializer::ToFile(Result, TABS_OPENED_FILE);
}

bool engine::editor::ScriptEditorUI::HasKeyboardFocus()
{
	return HasFocus;
}

void engine::editor::ScriptEditorUI::NavigateTo(std::string File, std::optional<ds::TokenPos> At)
{
	for (size_t i = 0; i < Tabs.size(); i++)
	{
		if (Tabs[i].Provider->EditedFile != File)
		{
			continue;
		}

		if (SelectedTab == i)
		{
			if (At)
			{
				Tabs[i].Provider->NavigateTo(EditorPosition(At->startPos, At->line),
					EditorPosition(At->endPos, At->line));
			}
			return;
		}
		else
		{
			OpenTab(i);
			if (At)
			{
				Tabs[i].Provider->NavigateTo(EditorPosition(At->startPos, At->line),
					EditorPosition(At->endPos, At->line));
			}
			return;
		}
	}

	AddTab(File);
	OpenTab(Tabs.size() - 1);
}

void engine::editor::ScriptEditorUI::OnChange(const string& Name)
{
	for (size_t i = 0; i < Tabs.size(); i++)
	{
		if (Tabs[i].Provider->EditedFile != Name)
		{
			continue;
		}

		if (Tabs[i].IsSaved)
		{
			Tabs[i].TabName->SetText(file::FileName(Name) + "*");
			Tabs[i].IsSaved = false;
		}
		break;
	}
}

void engine::editor::ScriptEditorUI::OnResized(Vec2f NewSize)
{
	this->Size = NewSize;

	if (UseVerticalTabs)
	{
		this->EditorBox->SetMinHeight(this->Size.Y - (76_px).GetScreen().Y);
		this->EditorBox->SetMinWidth(this->Size.X - (201_px).GetScreen().X);
	}
	else
	{
		this->EditorBox->SetMinHeight(this->Size.Y - (105_px).GetScreen().Y);
		this->EditorBox->SetMinWidth(this->Size.X - (11_px).GetScreen().X);
	}
	HorizontalTabBox->SetMinWidth(this->Size.X - (2_px).GetScreen().X);
	HorizontalTabBox->SetMaxWidth(this->Size.X - (2_px).GetScreen().X);
	SeparatorBackgrounds[0]->IsVisible = UseVerticalTabs;
	this->EditorBox->GetAbsoluteParent()->UpdateElement();
	UpdateTabSize(GetSelectedTab());
}

void engine::editor::ScriptEditorUI::UpdateColors()
{
	StatusText->SetColor(EditorUI::Theme.Text);
	for (auto& i : SeparatorBackgrounds)
	{
		i->SetColor(EditorUI::Theme.BackgroundHighlight);
	}

	if (IsFloating)
	{
		ScriptToolbar->SetToolbarColor(EditorUI::Theme.DarkBackground);
		VerticalTabBackground->SetColor(EditorUI::Theme.DarkBackground);
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

std::vector<kui::UIBox*> engine::editor::ScriptEditorUI::GetEditorBoxes()
{
	std::vector<kui::UIBox*> Result;

	for (auto& i : this->Tabs)
	{
		Result.push_back(i.Editor);
	}

	return Result;
}
