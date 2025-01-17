#pragma once
#include <Engine/Types.h>

namespace engine
{
	class Scene;
}

namespace engine::plugin
{
	struct PluginInfo
	{
		void* PluginHandle = nullptr;
		using SceneLoadFn = void(*)(Scene* New);

		SceneLoadFn OnNewSceneLoaded = nullptr;
	};

	void OnNewSceneLoaded(Scene* Target);

	void Load();
}