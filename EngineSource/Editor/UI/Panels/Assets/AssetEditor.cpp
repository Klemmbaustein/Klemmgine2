#include "AssetEditor.h"
#include <Editor/UI/Windows/MessageWindow.h>
#include <Editor/UI/EditorUI.h>

engine::editor::AssetEditor::AssetEditor(string NameFormat, AssetRef Asset)
	: EditorPanel(str::Format(NameFormat, Asset.DisplayName().c_str()), "")
{
	this->NameFormat = NameFormat;
	this->EditedAsset = Asset;

	AddShortcut(kui::Key::s, kui::Key::CTRL, [this]() {
		if (!Saved)
		{
			Save();
		}
	});
}

void engine::editor::AssetEditor::OnChanged()
{
	if (Saved)
	{
		UpdateName();
	}
}

void engine::editor::AssetEditor::Save()
{
	Saved = true;
	EditorUI::SetStatusMessage(str::Format("Saved file '%s'", EditedAsset.FilePath.c_str()),
		EditorUI::StatusType::Info);
	UpdateName();
}

void engine::editor::AssetEditor::Update()
{
	EditorPanel::Update();
}

bool engine::editor::AssetEditor::OnClosed()
{
	if (Saved)
		return true;

	new MessageWindow(str::Format("Save changes to '%s'?", EditedAsset.DisplayName().c_str()), "Unsaved changes", {
		IDialogWindow::Option{
			.Name = "Yes",
			.OnClicked = [this]() {
			Save();
			delete this;
		}},
		IDialogWindow::Option{
			.Name = "No",
			.OnClicked = [this]() {
			delete this;
		}},
		IDialogWindow::Option{
			.Name = "Cancel",
		} });
	return false;
}

void engine::editor::AssetEditor::UpdateName()
{
	if (Saved)
		SetName(str::Format(NameFormat, EditedAsset.DisplayName().c_str()));
	else
		SetName(str::Format(NameFormat, EditedAsset.DisplayName().c_str()) + "*");
}
