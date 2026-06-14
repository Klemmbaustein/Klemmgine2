#include "RenameWindow.h"
#include <DialogWindow.kui.hpp>
#include <Core/File/FileUtil.h>
#include <Engine/MainThread.h>
#include <Editor/UI/EditorUI.h>
#include <Editor/UI/Panels/AssetBrowser.h>
using namespace kui;

engine::editor::RenameWindow::RenameWindow(string File, std::function<void(string NewName)> OnRenamed, bool IsNewFile)
	: IDialogWindow(IsNewFile ? "Create file" : "Rename",
		{
			Option{.Name = IsNewFile ? "Create" : "Rename", .OnClicked = [this]() { Confirm(); Close(); }, .OnMainThread = false,},
			Option{ .Name = "Cancel", .Close = true, },
		}, Vec2ui(300, 130))
{
	this->IsNewFile = IsNewFile;
	this->File = File;
	this->OnRenamed = OnRenamed;
	Open();
}

void engine::editor::RenameWindow::Begin()
{
	IDialogWindow::Begin();

	auto RenameElement = new RenameWindowElement();

	if (IsNewFile)
	{
		RenameElement->SetFromString(str::Format("Type:  %s", file::Extension(File).c_str()));
		RenameElement->SetToString("Name:");
	}
	else
	{
		RenameElement->SetFromString(str::Format("From:  %s", file::FileNameWithoutExt(File).c_str()));
	}
	RenameElement->field->field->SetText(file::FileNameWithoutExt(File));
	EditField = RenameElement->field->field;
	EditField->SelectAll();

	Background->AddChild(RenameElement);
}

void engine::editor::RenameWindow::Update()
{
	if (!EditField->GetIsEdited())
		EditField->Edit();

	if (Popup->Input.IsKeyDown(Key::RETURN))
	{
		Confirm();
		Close();
	}
	else if (Popup->Input.IsKeyDown(Key::ESCAPE))
	{
		Close();
	}
}

void engine::editor::RenameWindow::Destroy()
{
	if (IsNewFile)
	{
		thread::ExecuteOnMainThread([File = this->File] {
			EditorUI::Instance->AssetsProvider->DeleteFile(File);
			EditorUI::ForEachPanel<AssetBrowser>([](AssetBrowser* i) {
				i->UpdateItems();
			});
		});
	}
}

void engine::editor::RenameWindow::Confirm()
{
	auto CopyOnRenamed = OnRenamed;
	string OutFile = EditField->GetText();
	string OldPath = File.substr(0, File.find_last_of("/") + 1);
	string OldExtension = File.substr(File.find_last_of(".") + 1);

	if (File.find(".") == std::string::npos)
		OutFile = OldPath + OutFile;
	else
		OutFile = OldPath + OutFile + "." + OldExtension;

	thread::ExecuteOnMainThread([CopyOnRenamed, OutFile]() {
		CopyOnRenamed(OutFile);
		});
}
