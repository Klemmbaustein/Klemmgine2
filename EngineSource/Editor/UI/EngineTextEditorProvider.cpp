#include "EngineTextEditorProvider.h"
#include <kui/UI/UITextEditor.h>
#include <kui/Window.h>

using namespace kui;
using namespace engine::editor;

engine::editor::EngineTextEditorProvider::EngineTextEditorProvider(std::string File)
	: FileEditorProvider(File)
{
	this->EditedFile = File;
}

engine::editor::EngineTextEditorProvider::~EngineTextEditorProvider()
{
}

void engine::editor::EngineTextEditorProvider::Update()
{
	auto Win = Window::GetActiveWindow();

	auto Pos = Win->Input.MousePosition;

	if (Win->Input.IsRMBClicked)
	{
		std::vector<DropdownMenu::Option> Options = GetRightClickOptions(ParentEditor->ScreenToEditor(Pos, false));

		new DropdownMenu(Options, Pos);

		if (ParentEditor->GetSelectedText().empty())
		{
			ParentEditor->SetCursorPosition(ParentEditor->ScreenToEditor(Pos));
		}
		OnRightClick();
	}
}

void engine::editor::EngineTextEditorProvider::TrimWhitespace(size_t IgnoreLine)
{
	bool Changed = false;
	for (size_t i = 0; i < Lines.size(); i++)
	{
		if (IgnoreLine == i)
		{
			continue;
		}

		auto& str = Lines[i];

		auto lastChar = str.find_last_not_of("\t ");

		if (lastChar == str.size())
		{
			continue;
		}

		auto sub = str.substr(0, lastChar + 1);

		if (ParentEditor->IsLineLoaded(i))
		{
			SetLine(i, { TextSegment(sub, TextColor) });
		}
		else
		{
			this->Lines[i] = sub;
		}
		Changed = true;
	}

	Commit();
}

std::vector<DropdownMenu::Option> engine::editor::EngineTextEditorProvider::GetRightClickOptions(EditorPosition At)
{
	std::vector<DropdownMenu::Option> Options;

	Options.push_back(DropdownMenu::Option{
		.Name = "Cut",
		.Shortcut = "Ctrl+X",
		.OnClicked = [this] {
			Window::GetActiveWindow()->Input.SetClipboard(ParentEditor->GetSelectedText());
			ParentEditor->DeleteSelection();
	} });

	Options.push_back(DropdownMenu::Option{
		.Name = "Copy",
		.Shortcut = "Ctrl+C",
		.OnClicked = [this] {
			Window::GetActiveWindow()->Input.SetClipboard(ParentEditor->GetSelectedText());
	} });

	Options.push_back(DropdownMenu::Option{
		.Name = "Paste",
		.Shortcut = "Ctrl+V",
		.OnClicked = [this] {
			ParentEditor->DeleteSelection();
			ParentEditor->Insert(Window::GetActiveWindow()->Input.GetClipboard(),
				ParentEditor->GetCursorPosition(), true);
		}
		});
	return Options;
}

void engine::editor::EngineTextEditorProvider::OnRightClick()
{
}
