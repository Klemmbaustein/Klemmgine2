#include "PluginLoader.h"
#include <Core/Platform/Platform.h>
#include <Core/File/TextSerializer.h>
#include <Engine/Scene.h>
#include <Engine/Subsystem/PluginSubsystem.h>
#include <filesystem>
#include <cstring>
#include <kui/Timer.h>

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

static char* StrDup(string From)
{
	char* NewString = (char*)malloc(From.size() + 1);
	if (!NewString)
		return nullptr;
	memcpy(NewString, From.data(), From.size());

	NewString[From.size()] = 0;
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

static const string PLUGIN_DIR = "Plugins/";

void engine::plugin::Load(subsystem::PluginSubsystem* System)
{
	if (!std::filesystem::exists(PLUGIN_DIR))
		return;

	for (const auto& i : std::filesystem::directory_iterator(PLUGIN_DIR))
	{
		if (std::filesystem::exists(i.path() / "Plugin.k2p"))
			TryLoadPlugin(i.path().string(), System);
	}
}

void engine::plugin::TryLoadPlugin(string Path, subsystem::PluginSubsystem* System)
{
	using namespace engine::internal::platform;
	using namespace engine::subsystem;
	using LogType = Subsystem::LogType;

	try
	{
		PluginInfo New;

		auto File = SerializedValue(TextSerializer::FromFile(Path + "/Plugin.k2p"));

		New.Name = File.At("name").GetString();

		System->Print(str::Format("Loading plugin: %s", New.Name.c_str()), LogType::Info);

		string PluginPath = File.At("binary").GetString();

#if WINDOWS
		string PluginBinary = str::Format("plugins/%s.dll", PluginPath.c_str());
#else
		string PluginBinary = str::Format("plugins/lib%s.so", PluginPath.c_str());
#endif

		SharedLibrary* Library = LoadSharedLibrary(PluginBinary);

		if (!Library)
		{
			System->Print(str::Format("Failed to load shared library file: %s", PluginPath.c_str()), LogType::Warning);
			return;
		}

		EnginePluginInterface TargetPluginInterface = ctx;
		TargetPluginInterface.PluginPath = StrDup(str::Format("Plugins/%s/", New.Name.c_str()));

		kui::Timer t;

		PluginLoadFn PluginLoad = (PluginLoadFn)GetLibraryFunction(Library, "PluginLoad");
		PluginLoad(&TargetPluginInterface);

		RegisterTypesFn RegisterTypes = (RegisterTypesFn)GetLibraryFunction(Library, "RegisterTypes");
		RegisterTypes();

		New.OnNewSceneLoaded = (PluginInfo::SceneLoadFn)GetLibraryFunction(Library, "OnSceneLoaded");
		New.PluginHandle = Library;

		System->Print(str::Format("Finished loading %s (in %ims)", New.Name.c_str(), int(t.Get() * 1000.0f)), LogType::Note);

		LoadedPlugins.push_back(New);
	}
	catch (SerializeException e)
	{
		System->Print(str::Format("Failed to load plugin: %s", e.what()), LogType::Warning);
	}
}
