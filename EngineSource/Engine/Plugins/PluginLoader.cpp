#include "PluginLoader.h"
#include <Core/File/FileUtil.h>
#include <Core/File/TextSerializer.h>
#include <Core/Platform/Platform.h>
#include <cstring>
#include <Editor/Editor.h>
#include <Engine/Console.h>
#include <Engine/Debug/TimeLogger.h>
#include <Engine/Input.h>
#include <Engine/MainThread.h>
#include <Engine/Objects/Components/MeshComponent.h>
#include <Engine/Plugins/PluginUI.h>
#include <Engine/Scene.h>
#include <Engine/Stats.h>
#include <Engine/Subsystem/PluginSubsystem.h>
#include <filesystem>
#include <kui/KlemmUI.h>

#include "InterfaceStruct.hpp"

using namespace engine;
using namespace engine::plugin;

// Incredibly cursed, but it means I wont have to write everything twice!

#undef STRUCT_MEMBER
#undef STRUCT_MEMBER_CALL_DIRECT
#define STRUCT_MEMBER(name, ret, args, func) .name = plugin::EnginePluginInterface:: name ## Fn ([] args -> ret { func ; }),
#define STRUCT_MEMBER_CALL_DIRECT(name, ret, args, func) .name = plugin::EnginePluginInterface:: name ## Fn (func),

static char* StrDup(engine::string From)
{
	char* NewString = (char*)malloc(From.size() + 1);
	if (!NewString)
		return nullptr;
	memcpy(NewString, From.data(), From.size());

	NewString[From.size()] = 0;
	return NewString;
}

static plugin::LogEntry* GetLog(size_t* OutSize)
{
	static std::vector<LogEntry> Entries;
	static std::vector<Log::Message> Messages;
	static std::vector<std::vector<LogPrefix>> Prefixes;

	Messages = Log::GetMessages();
	Entries.clear();
	Prefixes.resize(Messages.size());

	size_t Index = 0;
	for (auto& i : Messages)
	{
		Prefixes[Index].clear();
		for (auto& pref : i.Prefixes)
		{
			Prefixes[Index].push_back(LogPrefix{
				.Text = pref.Text.c_str(),
				.Color = pref.Color,
				});
		}

		Entries.push_back(LogEntry{
				.Prefixes = Prefixes[Index].data(),
				.PrefixSize = Prefixes[Index].size(),
				.Message = i.Message.c_str(),
				.Color = i.Color,
			});
		Index++;
	}

	*OutSize = Entries.size();
	return Entries.data();
}

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

void engine::plugin::Update()
{
	for (auto& i : LoadedPlugins)
	{
		if (i.PluginUpdate)
			i.PluginUpdate(stats::DeltaTime);
	}
}

void engine::plugin::Unload(subsystem::PluginSubsystem* System)
{
	using namespace engine::platform;

	for (auto& i : LoadedPlugins)
	{
		debug::TimeLogger PluginLoadTime{ str::Format("Unloaded plugin: %s", i.Name.c_str()), System->GetLogPrefixes() };
		if (i.PluginUnload)
			i.PluginUnload();
		UnloadSharedLibrary((SharedLibrary*)i.PluginHandle);
	}
	LoadedPlugins.clear();
}

void engine::plugin::TryLoadPlugin(string Path, subsystem::PluginSubsystem* System)
{
	using namespace engine::platform;
	using namespace engine::subsystem;
	using LogType = Subsystem::LogType;

	try
	{
		PluginInfo New;

		auto File = SerializedValue(TextSerializer::FromFile(Path + "/Plugin.k2p"));

		New.Name = File.At("name").GetString();

		string PluginFileName = File.At("binary").GetString();

		debug::TimeLogger PluginLoadTime{ str::Format("Successfully loaded plugin: %s", New.Name.c_str()), System->GetLogPrefixes() };

		string PluginPath = file::FilePath(platform::GetExecutablePath());

		if (std::filesystem::exists("Plugins/bin"))
		{
			PluginPath = "Plugins/bin";
		}
		else
		{
			PluginPath.append("/plugins");
		}

#if WINDOWS
		string PluginBinary = str::Format("%s/%s.dll", PluginPath.c_str(), PluginFileName.c_str());
#else
		string PluginBinary = str::Format("out/build/WSL-GCC-Release/bin/%s/lib%s.so", PluginPath.c_str(), PluginFileName.c_str());
#endif

		SharedLibrary* Library = LoadSharedLibrary(PluginBinary);

		if (!Library)
		{
			System->Print(str::Format("Failed to load shared library file: %s/%s", PluginPath.c_str(), PluginFileName.c_str()), LogType::Warning);
			return;
		}

		EnginePluginInterface TargetPluginInterface = ctx;
		TargetPluginInterface.PluginPath = StrDup(str::Format("Plugins/%s/", New.Name.c_str()));

		PluginLoadFn PluginLoad = PluginLoadFn(GetLibraryFunction(Library, "PluginLoad"));
		PluginLoad(&TargetPluginInterface);

		RegisterTypesFn RegisterTypes = RegisterTypesFn(GetLibraryFunction(Library, "RegisterTypes"));
		RegisterTypes();

		New.OnNewSceneLoaded = PluginInfo::SceneLoadFn(GetLibraryFunction(Library, "OnSceneLoaded"));
		New.PluginUpdate = PluginInfo::UpdateFn(GetLibraryFunction(Library, "Update"));
		New.PluginUnload = PluginInfo::PluginUnloadFn(GetLibraryFunction(Library, "PluginUnload"));
		New.PluginHandle = Library;

		LoadedPlugins.push_back(New);
	}
	catch (SerializeException& e)
	{
		System->Print(str::Format("Failed to load plugin %s: %s", Path.c_str(), e.what()), LogType::Warning);
	}
}
