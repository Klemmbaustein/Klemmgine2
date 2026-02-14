#include "CreateProjectWindow.h"
#include "LauncherProject.h"
#include <Engine/MainThread.h>
#include <Core/Platform/Platform.h>
#include <kui/App.h>
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
		}, Vec2ui(400, 300))
{
	this->OnAccept = OnAccept;

#if WINDOWS
	// Parity with Visual Studio source code path.
	this->Path = platform::GetSystemHomeDir() + "\\source\\Klemmgine";
#else
	this->Path = platform::GetSystemHomeDir() + "/src/Klemmgine";
#endif

	Open();
}

void engine::editor::launcher::CreateProjectWindow::Begin()
{
	IDialogWindow::Begin();
	Menu = new PropertyMenu(this->DefaultFont);
	Menu->SetMinWidth(350_px);
	Menu->SetMaxWidth(350_px);
	Background->AddChild(Menu);
	Background->SetHorizontalAlign(UIBox::Align::Centered);


	Menu->CreateNewHeading("Create new project");
	Menu->AddStringEntry("Name", this->ProjectName, nullptr);
	Menu->AddStringEntry("Path", this->Path, nullptr);
	Menu->AddButtonEntry("", "Browse", [this]() {
		string NewPath = kui::app::SelectFileDialog(true);
		if (!NewPath.empty())
		{
			this->Path = NewPath;
			Menu->UpdateProperties();
		}
	});

	auto ResultElement = Menu->CreateNewEntry("Result");

	ResultText = new UIText(12_px, EditorUI::Theme.Text, "", this->DefaultFont);
	ResultElement->valueBox->AddChild(ResultText
		->SetWrapEnabled(true, 200_px));
}

void engine::editor::launcher::CreateProjectWindow::Update()
{
#if WINDOWS
	ResultText->SetText(str::Format("Project Path: %s\nName: %s",
		(Path + "\\" + ProjectName.c_str()).c_str(), ProjectName.c_str()));
#else
	ResultText->SetText(str::Format("Project Path: %s\nName: %s",
		(Path + "/" + ProjectName.c_str()).c_str(), ProjectName.c_str()));
#endif
}

void engine::editor::launcher::CreateProjectWindow::Destroy()
{
}

void engine::editor::launcher::CreateProjectWindow::Accept()
{
	string ResultPath = this->Path + "/" + ProjectName;

	std::filesystem::create_directories(ResultPath + "/Assets");

	auto Projects = LauncherProject::GetProjects();

	Projects.push_back(LauncherProject{
		.Name = ProjectName,
		.Path = std::filesystem::canonical(ResultPath).string(),
		});

	LauncherProject::SaveProjects(Projects);

	thread::ExecuteOnMainThread(std::bind(OnAccept, ResultPath));
}
