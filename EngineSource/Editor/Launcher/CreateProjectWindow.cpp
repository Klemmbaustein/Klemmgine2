#include "CreateProjectWindow.h"
#include "LauncherProject.h"
#include <Engine/MainThread.h>
#include <filesystem>

using namespace kui;

engine::editor::launcher::CreateProjectWindow::CreateProjectWindow(std::function<void(std::string Path)> OnAccept)
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

void engine::editor::launcher::CreateProjectWindow::Begin()
{
	IDialogWindow::Begin();
	Element = new NewProjectWindowElement();
	Background->AddChild(Element);
}

void engine::editor::launcher::CreateProjectWindow::Update()
{
}

void engine::editor::launcher::CreateProjectWindow::Destroy()
{
}

void engine::editor::launcher::CreateProjectWindow::Accept()
{
	string Name = Element->nameField->field->GetText();
	string Path = "F:/Klemmgine/Projects/" + Name;

	std::filesystem::create_directories(Path + "/Assets");

	auto Projects = LauncherProject::GetProjects();

	Projects.push_back(LauncherProject{
		.Name = Name,
		.Path = Path,
		});

	LauncherProject::SaveProjects(Projects);

	thread::ExecuteOnMainThread(std::bind(OnAccept, Path));
}
