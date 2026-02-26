#include "EditorLauncher.h"
#include <Core/LaunchArgs.h>
#include <Engine/Internal/PlatformGraphics.h>
#include "CreateProjectWindow.h"
#include <Editor/Editor.h>
#include <Editor/Server/EditorServerSubsystem.h>
#include <Editor/UI/EditorUI.h>
#include <Editor/UI/Windows/SettingsWindow.h>
#include <Editor/Settings/EditorSettings.h>
#include <Engine/Version.h>
#include <Editor/UI/Windows/MessageWindow.h>
#include <Engine/Engine.h>
#include <Engine/MainThread.h>
#include <Core/File/FileUtil.h>
#include <kui/Window.h>
#include <filesystem>
#include <SDL3/SDL.h>

using namespace engine;
using namespace engine::editor;
using namespace kui;

engine::editor::launcher::EditorLauncher::EditorLauncher()
{
}

void engine::editor::launcher::EditorLauncher::InitWindow()
{
	EditorUI::InitTheme();
	SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO);
	thread::IsMainThread = true;

	editor::OpenEditorAt(std::filesystem::canonical("./Engine/").string());

	LauncherWindow = new Window("Klemmgine 2 Project Manager", Window::WindowFlag::Resizable,
		Window::POSITION_CENTERED, Vec2ui(700, 500));
	LauncherWindow->OnResizedCallback = std::bind(&EditorLauncher::OnWindowResized, this);

	WindowFont = new Font("res:DefaultFont.ttf");
	LauncherWindow->Markup.SetDefaultFont(WindowFont);

	auto& Theme = EditorUI::Theme;
	EditorUI::UpdateTheme(LauncherWindow, false);


	if (!launchArgs::GetArg("noWarning").has_value()
		&& Settings::GetInstance()->Interface.GetSetting("showDevWarning", true).GetBool())
	{
		new MessageWindow(R"(This is in development. This is not ready for proper use in a large project.

What is mostly ready:
- Scene logic, collision, physics.
- 3D rendering, shader and post processing system.
- UI and game logic scripting with an in editor IDE.

What isn't:
- No audio output or features yet.
- Some scripting language features (like enums, structs, generics) are only available to native code.

Existing features and APIs might change.)",
VersionInfo::Get().Build, {
	IDialogWindow::Option{
		.Name = "Don't show again",
		.OnClicked = [] {
			Settings::GetInstance()->Interface.SetSetting("showDevWarning", false);
			Settings::GetInstance()->Save();
		},
	},
	IDialogWindow::Option{
		.Name = "Ok",
		.IsAccept = true,
		.IsClose = true,
	},
			});
	}
}

void engine::editor::launcher::EditorLauncher::InitLayout()
{
	auto& Theme = EditorUI::Theme;

	Element = new LauncherElement();

	LauncherToolbar = new Toolbar(false, Theme.DarkBackground);

	LauncherToolbar->AddButton("New project", EditorUI::Asset("Plus.png"), [this]() {
		new CreateProjectWindow([this](std::string ProjectPath) {
			this->ProjectPathToLaunch = ProjectPath;
			LauncherWindow->Close();
			this->Result = LauncherResult::LaunchProject;
		});
	});
	LauncherToolbar->AddButton("Add existing project", EditorUI::Asset("ExitFolder.png"), [this]() {
		auto Result = platform::OpenFileDialog({
			platform::FileDialogFilter{
				.Name = "Klemmgine projects",
				.FileTypes = {"json"},
			},
			});

		if (Result.empty())
		{
			return;
		}
		auto Projects = LauncherProject::GetProjects();

		for (auto& i : Result)
		{
#if WINDOWS
			i = str::ReplaceChar(i, '\\', '/');
#endif

			Projects.push_back(LauncherProject{
				.Name = file::FileName(file::FilePath(i)),
				.Path = file::FilePath(i),
				});

		}
		LauncherProject::SaveProjects(Projects);
		this->UpdateProjectList();
	});

	//LauncherToolbar->AddButton("Add server", "", [this]() {
	//	new ServerConnectDialog([this](ConnectResult r) {
	//		if (r.Connect)
	//		{
	//			this->Connection = r;
	//			this->Result = LauncherResult::ConnectToServer;
	//			LauncherWindow->Close();
	//		}
	//	});
	//});

	LauncherToolbar->AddButton("Settings", EditorUI::Asset("Settings.png"), [this]() {
		new SettingsWindow();
	});

	Element->content->AddChild(LauncherToolbar);

	ProjectList = new UIScrollBox(false, 0, true);

	ProjectList->SetMinWidth(UISize::Parent(1));
	ProjectList->SetMinHeight(1);

	Element->content->AddChild(ProjectList);

	Element->openButton->IsCollapsed = true;
	Element->removeButton->IsCollapsed = true;

	Element->openButton->btn->OnClicked = [this]() {
		LauncherWindow->Close();
		ProjectPathToLaunch = SelectedProject->Path;
		this->Result = LauncherResult::LaunchProject;
	};

	Element->removeButton->btn->OnClicked = [this]() {
		auto& Selected = SelectedProject->Path;
		auto Projects = LauncherProject::GetProjects();

		for (auto i = Projects.begin(); i < Projects.end(); i++)
		{
			if (i->Path == Selected)
			{
				Projects.erase(i);
				break;
			}
		}
		LauncherProject::SaveProjects(Projects);
		this->UpdateProjectList();
	};

	Window::GetActiveWindow()->Markup.ListenToGlobal("Color_Background", AnyContainer(), this, [this]() {
		UpdateProjectList();
		LauncherToolbar->SetToolbarColor(EditorUI::Theme.DarkBackground);
	});

	OnWindowResized();
}

void engine::editor::launcher::EditorLauncher::ClearSelection()
{
	for (auto& i : Projects)
	{
		i->SetBorderSize(0);
		i->SetColor(EditorUI::Theme.LightBackground);
	}
}

void engine::editor::launcher::EditorLauncher::Run()
{
	InitWindow();
	InitLayout();

	UpdateProjectList();

	while (LauncherWindow->UpdateWindow())
	{
		thread::MainThreadUpdate();
	}

	delete WindowFont;
	delete LauncherWindow;

	switch (Result)
	{
	case engine::editor::launcher::LauncherResult::LaunchProject:
	{
		std::filesystem::current_path(ProjectPathToLaunch);

		auto Engine = Engine::Init();
		Engine->Run();
		break;
	}
	case engine::editor::launcher::LauncherResult::ConnectToServer:
	{
		auto Engine = Engine::Init();
		Engine->LoadSubsystem(new EditorServerSubsystem(this->Connection.Connection));
		Engine->Run();
		break;
	}
	break;
	case engine::editor::launcher::LauncherResult::Exit:
	default:
		return;
	}
}

void engine::editor::launcher::EditorLauncher::UpdateProjectList()
{
	Projects.clear();
	SelectedProject = {};

	Element->openButton->IsCollapsed = true;
	Element->removeButton->IsCollapsed = true;

	ProjectList->DeleteChildren();

	auto FoundProjects = LauncherProject::GetProjects();

	for (auto& i : FoundProjects)
	{
		auto Elem = new LauncherProjectElement();

		Elem->SetName(i.Name);
		Elem->SetDescription(i.Path);
		Elem->SetColor(EditorUI::Theme.LightBackground);
		Elem->SetBorderSize(0);
		Elem->btn->OnClicked = [this, Elem, i]() {
			ClearSelection();

			if (SelectedProject && SelectedProject->Path == i.Path)
			{
				ProjectPathToLaunch = i.Path;
				LauncherWindow->Close();
				Result = LauncherResult::LaunchProject;
			}

			Elem->SetBorderSize(1_px);
			Elem->SetColor(EditorUI::Theme.HighlightDark);
			SelectedProject = i;
			Element->openButton->IsCollapsed = false;
			Element->removeButton->IsCollapsed = false;
			Element->UpdateElement();
			Element->footer->RedrawElement();
		};
		Projects.push_back(Elem);

		ProjectList->AddChild(Elem);
	}
}

void engine::editor::launcher::EditorLauncher::OnWindowResized()
{
	Element->SetContentSize(UISize::Screen(2).GetScreen().Y - UISize::Pixels(41).GetScreen().Y);
}
