#pragma once
#include <Launcher.kui.hpp>
#include <kui/Window.h>

namespace engine::editor::launcher
{
	class EditorLauncher
	{
	public:

		EditorLauncher();

		void Run();

	private:

		void OnWindowResized();

		kui::Font* WindowFont = nullptr;
		LauncherElement* Element = nullptr;
		kui::Window* LauncherWindow = nullptr;
	};
}