#include "EditorLauncher.h"
#include <kui/Window.h>
#include <Engine/Internal/PlatformGraphics.h>
#include <Editor/UI/EditorUI.h>
#include <Editor/UI/Elements/Toolbar.h>

using namespace engine;
using namespace engine::editor;
using namespace kui;

engine::editor::launcher::EditorLauncher::EditorLauncher()
{
}

void engine::editor::launcher::EditorLauncher::Run()
{
	EditorUI::InitTheme();

	LauncherWindow = new Window("Klemmgine 2 Launcher", Window::WindowFlag::Resizable);

	LauncherWindow->OnResizedCallback = std::bind(&EditorLauncher::OnWindowResized, this);

	WindowFont = new Font("res:DefaultFont.ttf");
	LauncherWindow->Markup.SetDefaultFont(WindowFont);

	auto& Theme = EditorUI::Theme;
	EditorUI::UpdateTheme(LauncherWindow, false);

	platform::SetWindowTheming(Theme.DarkBackground, Theme.Text,
		Theme.Highlight1, Theme.CornerSize.Value > 0, LauncherWindow);

	Element = new LauncherElement();

	auto t = new Toolbar(false);

	t->AddButton("Settings", EditorUI::Asset("Settings.png"), nullptr);

	Element->content->AddChild(t);

	OnWindowResized();

	while (LauncherWindow->UpdateWindow())
	{

	}
	delete WindowFont;
	delete LauncherWindow;
}

void engine::editor::launcher::EditorLauncher::OnWindowResized()
{
	Element->SetContentSize(UISize::Screen(2).GetScreen().X - UISize::Pixels(121).GetScreen().X);
}
