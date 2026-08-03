#pragma once
#include <Engine/Subsystem/Subsystem.h>
#include <Engine/Plugins/PluginInfo.h>
#include <Engine/Subsystem/SceneSubsystem.h>

namespace engine::plugin
{
	class PluginSubsystem : public subsystem::Subsystem
	{
	public:
		PluginSubsystem();
		~PluginSubsystem() override;

		void LoadPlugin(string Path, string PluginDir);

		void Update() override;

	private:
		SceneSubsystem* LastSceneSystem = nullptr;
		std::vector<PluginInfo> LoadedPlugins;
	};
}