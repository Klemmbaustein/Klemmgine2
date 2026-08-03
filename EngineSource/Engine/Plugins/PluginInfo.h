#pragma once
#include <Engine/Scene.h>
#include <Core/Platform/Platform.h>

namespace engine::plugin
{
	struct PluginInfo
	{
		string Name;
		string LibraryName;
		platform::SharedLibrary* PluginHandle = nullptr;
		using SceneLoadFn = void(*)(Scene* New);
		using UpdateFn = void(*)(float Delta);
		using PluginUnloadFn = void(*)();

		SceneLoadFn OnNewSceneLoaded = nullptr;
		UpdateFn PluginUpdate = nullptr;
		PluginUnloadFn PluginUnload = nullptr;
	};
}