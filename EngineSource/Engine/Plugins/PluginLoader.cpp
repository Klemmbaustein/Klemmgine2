#include "PluginLoader.h"
#include <Engine/Objects/Reflection/ObjectReflection.h>
#include <Engine/Internal/Platform.h>
#include <Engine/Log.h>
#include <Engine/Scene.h>
#include <iostream>
#include <filesystem>
#include <Engine/File/TextSerializer.h>
#include <Engine/Subsystem/PluginSubsystem.h>

#include "InterfaceStruct.hpp"

using namespace engine;

// Incredibly cursed, but it means I wont have to write everything twice!

#undef STRUCT_MEMBER
#undef STRUCT_MEMBER_CALL_DIRECT
#define STRUCT_MEMBER(name, ret, args, func) .name = plugin::EnginePluginInterface:: name ## Fn ([] args -> ret { func ; }),
#define STRUCT_MEMBER_CALL_DIRECT(name, ret, args, func) .name = plugin::EnginePluginInterface:: name ## Fn (func),

static auto ctx = plugin::EnginePluginInterface{
#include "InterfaceDefines.hpp"
};

static char* StrDup(const char* From, size_t FromSize)
{
	char* NewString = (char*)malloc(FromSize + 1);
	if (!NewString)
		return nullptr;
	memcpy(NewString, From, FromSize);

	NewString[FromSize] = 0;
	return NewString;
}

std::vector<engine::plugin::PluginInfo> LoadedPlugins;

void engine::plugin::OnNewSceneLoaded(Scene* Target)
{
	for (auto& i : LoadedPlugins)
	{
		if (i.OnNewSceneLoaded)
			i.OnNewSceneLoaded(Target);
	}
}

void engine::plugin::Load(subsystem::PluginSubsystem* System)
{
	for (auto& i : std::filesystem::directory_iterator("Plugins/"))
	{
		if (!std::filesystem::exists(i.path() / "Plugin.k2p"))
			continue;
		TryLoadPlugin(i.path().string(), System);
	}
}

void engine::plugin::TryLoadPlugin(string Path, subsystem::PluginSubsystem* System)
{
	using namespace engine::internal::platform;
	using namespace engine::subsystem;

	try
	{
		PluginInfo New;

		auto File = SerializedValue(TextSerializer::FromFile(Path + "/Plugin.k2p"));

		New.Name = File.At("name").GetString();

		System->Print(str::Format("Loading plugin: %s", New.Name.c_str()), Subsystem::LogType::Info);

		string PluginPath = File.At("binary").GetString();

#if WINDOWS
		SharedLibrary* Library = LoadSharedLibrary(str::Format("plugins/%s.dll", PluginPath.c_str()));
#else
		SharedLibrary* Library = LoadSharedLibrary(str::Format("plugins/lib%s.so", PluginPath.c_str()));
#endif

		if (!Library)
		{
			System->Print(str::Format("Failed to load shared library file: %s", PluginPath.c_str()), Subsystem::LogType::Warning);
			return;
		}

		EnginePluginInterface TargetPluginInterface = ctx;
		string Path = str::Format("Plugins/%s/", New.Name.c_str());
		TargetPluginInterface.PluginPath = StrDup(Path.c_str(), Path.size());

		PluginLoadFn PluginLoad = (PluginLoadFn)GetLibraryFunction(Library, "PluginLoad");
		PluginLoad(&TargetPluginInterface);

		RegisterTypesFn RegisterTypes = (RegisterTypesFn)GetLibraryFunction(Library, "RegisterTypes");
		RegisterTypes();

		New.OnNewSceneLoaded = (PluginInfo::SceneLoadFn)GetLibraryFunction(Library, "OnSceneLoaded");
		New.PluginHandle = Library;
		LoadedPlugins.push_back(New);
	}
	catch (SerializeException e)
	{
		System->Print(str::Format("Failed to load plugin: %s", e.what()), Subsystem::LogType::Warning);
	}
}
