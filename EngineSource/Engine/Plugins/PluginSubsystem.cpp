#include "PluginSubsystem.h"
#include <filesystem>
#include <Editor/Editor.h>
#include <Engine/Debug/TimeLogger.h>
#include <Engine/Plugins/PluginFunctions.h>
#include <Engine/Stats.h>
#include <Core/Platform/Platform.h>
#include <Core/File/TextSerializer.h>

using namespace engine::platform;
using namespace engine::plugin;
using namespace engine;

engine::plugin::PluginSubsystem::PluginSubsystem()
	: Subsystem("Plugin", Log::LogColor::Gray)
{
#ifdef EDITOR
	PluginDir = editor::GetEditorPath() + "/../Plugins/";
#else
	PluginDir = "Plugins/";
#endif

	if (!std::filesystem::exists(PluginDir))
		return;

	for (const auto& i : std::filesystem::directory_iterator(PluginDir))
	{
		if (std::filesystem::exists(i.path() / "Plugin.k2p"))
			LoadPluginData(i.path().string());
	}
}

engine::plugin::PluginSubsystem::~PluginSubsystem()
{
	for (auto& i : LoadedPlugins)
	{
		UnloadPlugin(&i);
	}
	LoadedPlugins.clear();
}

void engine::plugin::PluginSubsystem::LoadPluginData(string Path)
{
	PluginInfo New;

	auto File = SerializedValue(TextSerializer::FromFile(Path + "/Plugin.k2p"));

	New.Name = File.At("name").GetString();
	New.LibraryName = File.At("binary").GetString();
	New.IsDev = File.Contains("devOnly") && File.At("devOnly").GetBool();

	InitializePlugin(&New, this, PluginDir);

	LoadedPlugins.push_back(New);
}

void engine::plugin::PluginSubsystem::Update()
{
	if (this->LastSceneSystem != SceneSubsystem::Current)
	{
		if (SceneSubsystem::Current)
		{
			SceneSubsystem::Current->OnSceneLoaded.Add(this, [this](Scene* Target) {
				for (auto& i : this->LoadedPlugins)
				{
					if (i.OnNewSceneLoaded)
						i.OnNewSceneLoaded(Target);
				}
			});
		}
		this->LastSceneSystem = SceneSubsystem::Current;
	}

	for (auto& i : LoadedPlugins)
	{
		if (i.PluginUpdate)
			i.PluginUpdate(stats::DeltaTime);
	}
}

PluginInfo* engine::plugin::PluginSubsystem::GetPluginFromName(string Name)
{
	for (auto& i : this->LoadedPlugins)
	{
		if (i.Name == Name)
		{
			return &i;
		}
	}

	return nullptr;
}

void engine::plugin::PluginSubsystem::LoadPlugin(PluginInfo* Info)
{
	InitializePlugin(Info, this, PluginDir);
}

void engine::plugin::PluginSubsystem::UnloadPlugin(PluginInfo* Info)
{
	debug::TimeLogger PluginLoadTime{ str::Format("Unloaded plugin: %s", Info->Name.c_str()), GetLogPrefixes() };
	if (Info->PluginUnload)
		Info->PluginUnload();
	UnloadSharedLibrary(Info->PluginHandle);
	Info->PluginHandle = nullptr;
	Info->PluginUnload = nullptr;
	Info->PluginUpdate = nullptr;
	Info->OnNewSceneLoaded = nullptr;
}
