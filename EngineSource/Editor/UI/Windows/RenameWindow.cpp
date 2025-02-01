#ifdef EDITOR
#include "RenameWindow.h"
#include <DialogWindow.kui.hpp>
#include <Core/File/FileUtil.h>
#include <Engine/MainThread.h>
using namespace kui;

engine::editor::RenameWindow::RenameWindow(string File, std::function<void(string NewName)> OnRenamed)
	: IDialogWindow("Rename",
		{
			Option{ .Name = "Rename", .OnClicked = [this]() { Confirm(); }, .Close = true, },
			Option{ .Name = "Cancel", .Close = true, },
		}, Vec2ui(400, 150))
{
	this->File = File;
	this->OnRenamed = OnRenamed;
	Open();
}

void engine::editor::RenameWindow::Begin()
{
	IDialogWindow::Begin();

	auto RenameElement = new RenameWindowElement();

	RenameElement->SetFromString(str::Format("From:  %s", file::FileNameWithoutExt(File).c_str()));
	RenameElement->field->field->SetText(file::FileNameWithoutExt(File));
	EditField = RenameElement->field->field;

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

#endif
