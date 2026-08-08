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

		void LoadPluginData(string Path);

		void Update() override;

		std::vector<PluginInfo> LoadedPlugins;

		PluginInfo* GetPluginFromName(string Name);

		void LoadPlugin(PluginInfo* Info);
		void UnloadPlugin(PluginInfo* Info);
	private:
		string PluginDir;

		SceneSubsystem* LastSceneSystem = nullptr;
	};
}