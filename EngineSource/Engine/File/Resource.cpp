#include "Resource.h"
#include <map>
#include <functional>
#include <Core/Log.h>
#include <filesystem>
#include <kui/Resource.h>
#include <Engine/File/ArchiveResourceSource.h>
#include <Engine/File/EmbeddedResourceSource.h>
#include <set>
#include <Editor/Editor.h>

using namespace engine;
using namespace engine::resource;

static std::map<string, std::function<string(string, string)>> KnownFilePaths =
{
	{ "res:", [](string Path, string Pref) {
		return Path;
	} },
	{ "file:", [](string Path, string Pref) {
		return Path.substr(Pref.size());
	} },
};

static std::set<ResourceSource*> Sources;

string engine::resource::GetTextFile(string EnginePath)
{
	auto f = GetBinaryFile(EnginePath);

	if (!f)
	{
		return "";
	}

	string FileContent;
	f->ReadAll(FileContent);

	delete f;

	return FileContent;
}

string engine::resource::ConvertFilePath(string EnginePath, bool AllowFile)
{
	string FullPath;

	for (const auto& [Prefix, Convert] : KnownFilePaths)
	{
		if (EnginePath.substr(0, Prefix.size()) != Prefix)
		{
			continue;
		}

		if (Prefix == "file:" && !AllowFile)
		{
			continue;
		}

		FullPath = Convert(EnginePath, Prefix);
	}

	auto Found = LoadedAssets.find(EnginePath);

	if (Found != LoadedAssets.end())
	{
		return Found->second;
	}
	return EnginePath;
}

bool engine::resource::FileExists(string EnginePath)
{
	string Converted = EnginePath;

	for (auto& i : Sources)
	{
		if (i->FileExists(EnginePath))
		{
			return true;
		}
	}

	return kui::resource::FileExists(EnginePath) || (std::filesystem::exists(Converted) && AllowLocalFiles);
}

IBinaryStream* engine::resource::GetBinaryFile(string EnginePath)
{
	string FoundPrefix;

	for (const auto& [Prefix, Convert] : KnownFilePaths)
	{
		if (EnginePath.substr(0, Prefix.size()) != Prefix)
		{
			continue;
		}
		FoundPrefix = Prefix;
	}

	if (FoundPrefix == "res:")
	{
		auto res = kui::resource::GetBinaryResource(EnginePath.substr(FoundPrefix.size()));

		return new ReadOnlyBufferStream((const uByte*)res.Data, res.FileSize, false);
	}
	if (FoundPrefix.empty())
	{
		std::string FilePath = EnginePath;

		for (auto& i : Sources)
		{
			if (i->FileExists(EnginePath))
			{
				return i->GetFile(EnginePath);
			}
		}

		bool AllowFiles = AllowLocalFiles;

#ifdef EDITOR
		// Allow loading local files belonging to the editor
		AllowFiles = AllowLocalFiles || (EnginePath.starts_with(editor::GetEditorPath()) && EnginePath.find("..") == string::npos);
#endif
		if (AllowFiles && std::filesystem::exists(FilePath)
			&& !std::filesystem::is_directory(FilePath))
		{
			return new FileStream(FilePath, true);
		}
	}

	return nullptr;
}

void engine::resource::RemoveListener(void* Target)
{
	for (auto& i : AssetListeners)
	{
		i.second.Remove(Target);
	}
}

void resource::LoadSceneFiles(string ScenePath)
{
	for (auto& i : Sources)
	{
		i->LoadSceneFiles(ScenePath);
	}
}

void resource::ScanForAssets()
{
	using std::filesystem::recursive_directory_iterator;

	LoadedAssets.clear();

	static bool ArchiveProviderLoaded = false;

	if (AllowLocalFiles && !ArchiveProviderLoaded)
	{
		ArchiveProviderLoaded = true;

		AddResourceSource(new EmbeddedResourceSource());
		if (std::filesystem::exists("Assets/archmap.bin"))
		{
			AddResourceSource(new ArchiveResourceSource());
		}
	}

	for (auto& i : Sources)
	{
		auto SourceAssets = i->GetFiles();

		for (auto& asset : SourceAssets)
		{
			LoadedAssets.insert(asset);
		}
	}
	if (AllowLocalFiles)
	{
		for (const auto& i : recursive_directory_iterator("Assets/"))
		{
			if (i.is_regular_file())
				LoadedAssets.insert(
					{
						i.path().filename().string(),
						str::ReplaceChar(i.path().string(), '\\', '/')
					});
		}
	}
}

void resource::AddResourceSource(ResourceSource* Source)
{
	Sources.insert(Source);
}

void resource::RemoveResourceSource(ResourceSource* Source)
{
	Sources.erase(Source);
}

std::map<string, string> resource::LoadedAssets;
std::map<string, Event<>> resource::AssetListeners;
bool resource::AllowLocalFiles = true;
