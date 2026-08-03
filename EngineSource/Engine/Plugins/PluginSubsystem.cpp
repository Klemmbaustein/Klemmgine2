#include "PluginSubsystem.h"
#include <filesystem>
#include <Editor/Editor.h>
#include <Engine/Debug/TimeLogger.h>
#include <Engine/Plugins/PluginFunctions.h>
#include <Engine/Stats.h>
#include <Core/Platform/Platform.h>
#include <Core/File/TextSerializer.h>

using namespace engine::platform;

engine::plugin::PluginSubsystem::PluginSubsystem()
	: Subsystem("Plugin", Log::LogColor::Gray)
{
#ifdef EDITOR
	static const string PLUGIN_DIR = editor::GetEditorPath() + "/../Plugins/";
#else
	static const string PLUGIN_DIR = "Plugins/";
#endif
	if (!std::filesystem::exists(PLUGIN_DIR))
		return;

	for (const auto& i : std::filesystem::directory_iterator(PLUGIN_DIR))
	{
		if (std::filesystem::exists(i.path() / "Plugin.k2p"))
			LoadPlugin(i.path().string(), PLUGIN_DIR);
	}
}

engine::plugin::PluginSubsystem::~PluginSubsystem()
{
	for (auto& i : LoadedPlugins)
	{
		debug::TimeLogger PluginLoadTime{ str::Format("Unloaded plugin: %s", i.Name.c_str()), GetLogPrefixes() };
		if (i.PluginUnload)
			i.PluginUnload();
		UnloadSharedLibrary((SharedLibrary*)i.PluginHandle);
	}
	LoadedPlugins.clear();
}

void engine::plugin::PluginSubsystem::LoadPlugin(string Path, string PluginDir)
{
	PluginInfo New;

	auto File = SerializedValue(TextSerializer::FromFile(Path + "/Plugin.k2p"));

	New.Name = File.At("name").GetString();
	New.LibraryName = File.At("binary").GetString();

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
