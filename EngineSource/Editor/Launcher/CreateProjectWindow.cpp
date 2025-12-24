#include "CreateProjectWindow.h"
#include <Engine/MainThread.h>

using namespace kui;

engine::editor::CreateProjectWindow::CreateProjectWindow(std::function<void(std::string Path)> OnAccept)
	: IDialogWindow("Create new project", {
	Option{
			.Name = "Create",
			.OnClicked = std::bind(&CreateProjectWindow::Accept, this),
			.Close = true,
			.IsAccept = true,
			.OnMainThread = false,
		},
	Option{
			.Name = "Cancel",
			.Close = true,
			.IsClose = true,
		}
		}, Vec2ui(640, 480))
{
	this->OnAccept = OnAccept;

	Open();
}

void engine::editor::CreateProjectWindow::Begin()
{
	IDialogWindow::Begin();
	Element = new NewProjectWindowElement();
	Background->AddChild(Element);
}

void engine::editor::CreateProjectWindow::Update()
{
}

void engine::editor::CreateProjectWindow::Destroy()
{
}

void engine::editor::CreateProjectWindow::Accept()
{
	thread::ExecuteOnMainThread(std::bind(OnAccept, Element->nameField->field->GetText()));
}
