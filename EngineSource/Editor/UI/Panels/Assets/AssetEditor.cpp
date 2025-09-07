#include "AssetEditor.h"
#include <Engine/Input.h>
#include <Editor/UI/EditorUI.h>
#include <Editor/UI/Windows/MessageWindow.h>
#include "TextEditorPanel.h"

engine::editor::AssetEditor::AssetEditor(string NameFormat, AssetRef Asset)
	: EditorPanel(str::Format(NameFormat, Asset.DisplayName().c_str()), "")
{
	this->NameFormat = NameFormat;
	this->EditedAsset = Asset;
}

void engine::editor::AssetEditor::OnChanged()
{
	if (Saved)
	{
		Saved = false;
		UpdateName();
	}
}

void engine::editor::AssetEditor::Save()
{
	Saved = true;
	UpdateName();
}

void engine::editor::AssetEditor::Update()
{
	EditorPanel::Update();
	if (!Saved && EditorUI::FocusedPanel == this
		&& input::IsKeyDown(input::Key::LCTRL) && input::IsKeyDown(input::Key::s))
	{
		Save();
	}
}

bool engine::editor::AssetEditor::OnClosed()
{
	if (Saved)
		return true;

	new MessageWindow(str::Format("Save changes to '%s'?", EditedAsset.DisplayName().c_str()), "Unsaved changes", {
	IDialogWindow::Option
	{
		.Name = "Yes",
		.OnClicked = [this]()
	{
		Save();
		delete this;
	}
	},
	IDialogWindow::Option
	{
		.Name = "No",
		.OnClicked = [this]()
	{
		delete this;
	}
	},
	IDialogWindow::Option
	{
		.Name = "Cancel",
	},
		});
	return false;
}

void engine::editor::AssetEditor::UpdateName()
{
	if (Saved)
		SetName(str::Format(NameFormat, EditedAsset.DisplayName().c_str()));
	else
		SetName(str::Format(NameFormat, EditedAsset.DisplayName().c_str()) + "*");
}
