#pragma once
#include <Core/Types.h>

namespace engine
{
	class Scene;
	namespace subsystem
	{
		class PluginSubsystem;
	}
}

namespace engine::plugin
{
	struct PluginInfo
	{
		string Name;
		void* PluginHandle = nullptr;
		using SceneLoadFn = void(*)(Scene* New);
		using UpdateFn = void(*)(float Delta);
		using PluginUnloadFn = void(*)();

		SceneLoadFn OnNewSceneLoaded = nullptr;
		UpdateFn PluginUpdate = nullptr;
		PluginUnloadFn PluginUnload = nullptr;
	};

	void OnNewSceneLoaded(Scene* Target);

	void Load(subsystem::PluginSubsystem* System);
	void Update();
	void Unload(subsystem::PluginSubsystem* System);

	void TryLoadPlugin(string Path, subsystem::PluginSubsystem* System);
}