#ifdef EDITOR_PLUGIN_SUPPORT
#include "PluginManagerWindow.h"
#include <PluginWindow.kui.hpp>
#include <Engine/Engine.h>
#include <Engine/Plugins/PluginSubsystem.h>
#include <Engine/MainThread.h>
#include <Editor/UI/Elements/Toolbar.h>

engine::editor::PluginManagerWindow::PluginManagerWindow()
	: IDialogWindow("Project Plugins", { Option{.Name = "Close", .IsClose = true} }, kui::Vec2ui(640, 480))
{
	auto PluginSysData = Engine::Instance->GetSubsystem<plugin::PluginSubsystem>();

	if (PluginSysData)
	{
		for (auto& i : PluginSysData->LoadedPlugins)
		{
			this->Plugins.push_back(PluginData{
				.Name = i.Name,
				.IsDev = i.IsDev,
				.IsExperimental = false,
				});
		}
	}

	Open();
}

void engine::editor::PluginManagerWindow::Begin()
{
	IDialogWindow::Begin();
	Background->SetHorizontal(false);

	auto t = new Toolbar(false, EditorUI::Theme.DarkBackground);

	t->AddButton("Refresh", EditorUI::Asset("Reload.png"), [] {

	});

	Background->AddChild(t);

	for (auto& i : this->Plugins)
	{
		auto Item = new PluginWindowItem();

		Item->SetName(i.Name);
		Item->devBadge->IsCollapsed = !i.IsDev;

		Background->AddChild(Item);

		Item->btn->OnClicked = [Name = i.Name] {
			thread::ExecuteOnMainThread([Name] {
				auto PluginSysData = Engine::Instance->GetSubsystem<plugin::PluginSubsystem>();

				PluginSysData->UnloadPlugin(PluginSysData->GetPluginFromName(Name));
			});
		};
	}
}

void engine::editor::PluginManagerWindow::Update()
{
}

void engine::editor::PluginManagerWindow::Destroy()
{
}
#endif