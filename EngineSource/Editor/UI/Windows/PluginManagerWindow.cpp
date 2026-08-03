#ifdef EDITOR_PLUGIN_SUPPORT
#include "PluginManagerWindow.h"
#include <PluginWindow.kui.hpp>

engine::editor::PluginManagerWindow::PluginManagerWindow()
	: IDialogWindow("Project Plugins", { Option{.Name = "Close", .IsClose = true} }, kui::Vec2ui(640, 480))
{
	Open();
}

void engine::editor::PluginManagerWindow::Begin()
{
	IDialogWindow::Begin();
	Background->AddChild(new PluginWindowItem());
}

void engine::editor::PluginManagerWindow::Update()
{
}

void engine::editor::PluginManagerWindow::Destroy()
{
}
#endif