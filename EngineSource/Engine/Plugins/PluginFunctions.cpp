#include "PluginFunctions.h"
#include <Core/File/FileUtil.h>
#include <Core/File/TextSerializer.h>
#include <Core/Platform/Platform.h>
#include <cstring>
#include <Editor/Editor.h>
#include <Engine/Console.h>
#include <Engine/Objects/SceneObject.h>
#include <Engine/Debug/TimeLogger.h>
#include <Engine/Input.h>
#include <Engine/MainThread.h>
#include <Engine/Objects/Components/MeshComponent.h>
#include <Engine/Plugins/PluginUI.h>
#include <Engine/Scene.h>
#include <Engine/Stats.h>
#include <Engine/Plugins/PluginSubsystem.h>
#include <kui/KlemmUI.h>
#include <filesystem>
#include <InterfaceStruct.hpp>

using namespace engine;
using namespace engine::plugin;
using namespace engine::platform;

// Incredibly cursed, but it means I won't have to write everything twice!

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

EnginePluginInterface engine::plugin::PluginInterface = EnginePluginInterface{
#include "InterfaceDefines.hpp"
};

void engine::plugin::InitializePlugin(PluginInfo* ToInitialize, PluginSubsystem* System, string PluginDir)
{
	debug::TimeLogger PluginLoadTime{ str::Format("Successfully loaded plugin: %s",
		ToInitialize->Name.c_str()), System->GetLogPrefixes() };

	string PluginPath = file::FilePath(platform::GetExecutablePath());
	string PluginFilesPath = file::FilePath(platform::GetExecutablePath());

	if (std::filesystem::exists("Plugins/bin"))
	{
		PluginPath = "Plugins/bin";
	}
	else
	{
		PluginPath.append("/plugins");
	}

#if LINUX
	const char* Found = getenv("LD_LIBRARY_PATH");

	if (!Found)
	{
		Found = "";
	}

	setenv("LD_LIBRARY_PATH", str::Format("%s;%s", PluginPath.c_str(), Found).c_str(), true);
#endif

#if WINDOWS
	string PluginBinary = str::Format("%s/%s.dll", PluginPath.c_str(), ToInitialize->LibraryName.c_str());
#else
	string PluginBinary = str::Format("%s/lib%s.so", PluginPath.c_str(), ToInitialize->LibraryName.c_str());
#endif

	SharedLibrary* Library = LoadSharedLibrary(PluginBinary);

	if (!Library)
	{
		System->Print(str::Format("Failed to load shared library file: %s/%s", PluginPath.c_str(), ToInitialize->LibraryName.c_str()),
			PluginSubsystem::LogType::Warning);
		PluginLoadTime.Cancel();
		return;
	}

	EnginePluginInterface TargetPluginInterface = PluginInterface;
	TargetPluginInterface.PluginPath = StrDup(str::Format("%s%s/", PluginDir.c_str(), ToInitialize->Name.c_str()));

	PluginLoadFn PluginLoad = PluginLoadFn(GetLibraryFunction(Library, "PluginLoad"));
	PluginLoad(&TargetPluginInterface);

	RegisterTypesFn RegisterTypes = RegisterTypesFn(GetLibraryFunction(Library, "RegisterTypes"));
	RegisterTypes();

	ToInitialize->OnNewSceneLoaded = PluginInfo::SceneLoadFn(GetLibraryFunction(Library, "OnSceneLoaded"));
	ToInitialize->PluginUpdate = PluginInfo::UpdateFn(GetLibraryFunction(Library, "Update"));
	ToInitialize->PluginUnload = PluginInfo::PluginUnloadFn(GetLibraryFunction(Library, "PluginUnload"));
	ToInitialize->PluginHandle = Library;

}
