#pragma once
#include <Launcher.kui.hpp>
#include <kui/Window.h>
#include <Editor/Server/ServerConnectDialog.h>
#include "LauncherProject.h"
#include <Editor/UI/Elements/Toolbar.h>

namespace engine::editor::launcher
{
	enum class LauncherResult
	{
		LaunchProject,
		ConnectToServer,
		Exit,
	};

	class EditorLauncher
	{
	public:

		EditorLauncher();

		void Run();

	private:

		void UpdateProjectList();

		void OnWindowResized();

		void InitWindow();
		void InitLayout();

		void ClearSelection();

		kui::Font* WindowFont = nullptr;
		LauncherElement* Element = nullptr;
		kui::UIScrollBox* ProjectList = nullptr;
		kui::Window* LauncherWindow = nullptr;

		Toolbar* LauncherToolbar = nullptr;

		std::vector<LauncherProjectElement*> Projects;
		std::optional<LauncherProject> SelectedProject;

		string ProjectPathToLaunch;
		LauncherResult Result = LauncherResult::Exit;
		ConnectResult Connection;
	};
}
