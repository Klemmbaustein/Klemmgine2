#include "PluginLoader.h"
#include <Engine/Objects/Reflection/ObjectReflection.h>
#include <Engine/Internal/Platform.h>
#include <Engine/Log.h>
#include <Engine/Scene.h>
#include <iostream>
#include <filesystem>

#include "InterfaceStruct.hpp"

using namespace engine;

// Incredibly cursed, but it means I wont have to write everything twice!

#undef STRUCT_MEMBER
#undef STRUCT_MEMBER_CALL_DIRECT
#define STRUCT_MEMBER(name, ret, args, func) .name = [] args -> ret { func ; },
#define STRUCT_MEMBER_CALL_DIRECT(name, ret, args, func) .name = func,

static auto ctx = plugin::EnginePluginInterface{
#include "InterfaceDefines.hpp"
};

std::vector<engine::plugin::PluginInfo> LoadedPlugins;

void engine::plugin::OnNewSceneLoaded(Scene* Target)
{
	for (auto& i : LoadedPlugins)
	{
		if (i.OnNewSceneLoaded)
			i.OnNewSceneLoaded(Target);
	}
}

void engine::plugin::Load()
{
	using namespace engine::internal::platform;

	PluginInfo New;

	Log::Info("Loading plugin: C#");

#if WINDOWS
	SharedLibrary* Library = LoadSharedLibrary("Klemmgine.CSharp.Loader.dll");
#else
	SharedLibrary* Library = LoadSharedLibrary("libKlemmgine.CSharp.Loader.so");
#endif

	if (!Library)
	{
		Log::Error("Failed to load library file");
		return;
	}

	EnginePluginInterface TargetPluginInterface = ctx;
	TargetPluginInterface.PluginPath = "Plugins/CSharp/";

	PluginLoadFn PluginLoad = (PluginLoadFn)GetLibraryFunction(Library, "PluginLoad");
	PluginLoad(&TargetPluginInterface);

	RegisterTypesFn RegisterTypes = (RegisterTypesFn)GetLibraryFunction(Library, "RegisterTypes");
	RegisterTypes();

	New.OnNewSceneLoaded = (PluginInfo::SceneLoadFn)GetLibraryFunction(Library, "OnSceneLoaded");
	New.PluginHandle = Library;
	LoadedPlugins.push_back(New);
}
