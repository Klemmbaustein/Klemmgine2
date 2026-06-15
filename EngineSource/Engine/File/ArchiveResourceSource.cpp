#include "ArchiveResourceSource.h"
#include <Core/Log.h>
#include <set>
#include <Core/File/FileUtil.h>
#include <Core/File/BinarySerializer.h>

using namespace engine;

engine::resource::ArchiveResourceSource::ArchiveResourceSource()
{
	if (LoadedArchives.empty())
	{
		LoadArchive("scenes");
		LoadArchive("scripts");
		LoadArchive("shaders");
	}

	SceneArchives.clear();

	auto ArchiveMap = BinarySerializer::FromFile("Assets/archmap.bin", "archm");

	for (auto& i : ArchiveMap)
	{
		std::vector<string> Scenes;

		for (auto& scn : i.Value.GetArray())
		{
			Scenes.push_back(scn.GetString());
		}

		FileStream Stream = FileStream(ArchiveNameToPath(i.Name), true);
		auto Files = Archive::GetAssets(&Stream);
		for (auto& f : Files)
		{
			ArchiveAssets.insert({ file::FileName(f), f });
			FileMap.insert({f, i.Name});
		}

		SceneArchives.insert({ i.Name, Scenes });
	}
}

bool engine::resource::ArchiveResourceSource::FileExists(string Path)
{
	return FileMap.contains(Path);
}

IBinaryStream* engine::resource::ArchiveResourceSource::GetFile(string Path)
{
	for (auto& i : LoadedArchives)
	{
		ReadOnlyBufferStream* f = i.second->GetFile(Path);
		if (f)
			return f;
	}

	auto found = this->FileMap.find(Path);

	if (found != this->FileMap.end())
	{
		FileStream Stream = FileStream(ArchiveNameToPath(found->second), true);
		return Archive::StreamAsset(&Stream, Path);
	}

	return nullptr;
}

std::map<string, string> engine::resource::ArchiveResourceSource::GetFiles()
{
	return ArchiveAssets;
}

void engine::resource::ArchiveResourceSource::LoadSceneFiles(string ScenePath)
{
	std::set<string> ArchivesToLoad;

	for (auto& [Archive, DependentScenes] : SceneArchives)
	{
		for (auto& scn : DependentScenes)
		{
			if (scn == ScenePath)
			{
				ArchivesToLoad.insert(Archive);
			}
		}
	}

	std::vector<string> ArchivesToRemove;
	for (auto& [Name, _] : LoadedArchives)
	{
		// The scenes and scripts archives are always loaded.
		if (Name == "scenes" || Name == "scripts" || Name == "shaders")
			continue;

		if (!ArchivesToLoad.contains(Name))
		{
			ArchivesToRemove.push_back(Name);
		}
		else
		{
			ArchivesToLoad.erase(Name);
		}
	}

	for (const string& i : ArchivesToRemove)
	{
		UnloadArchive(i);
	}

	for (const string& i : ArchivesToLoad)
	{
		LoadArchive(i);
	}

	resource::ScanForAssets();
}

void engine::resource::ArchiveResourceSource::LoadArchive(string Name)
{
	if (LoadedArchives.contains(Name))
		return;

	Log::Note(str::Format("Loading archive: %s", Name.c_str()));

	Archive* NewArchive = new Archive(ArchiveNameToPath(Name));

	LoadedArchives.insert({ Name, NewArchive });
}

void engine::resource::ArchiveResourceSource::UnloadArchive(string Name)
{
	Log::Note(str::Format("Unloading archive: %s", Name.c_str()));
	LoadedArchives.erase(Name);
}