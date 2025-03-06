#include "Resource.h"
#include <map>
#include <functional>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <kui/Resource.h>
#include <Core/File/BinarySerializer.h>
#include <Core/Archive/Archive.h>
#include <mutex>
#include <Core/Log.h>
#include <set>

using namespace engine;
using namespace engine::resource;

static std::map<string, std::function<string(string, string)>> KnownFilePaths =
{
	{ "res:", [](string Path, string Pref) {
		return Path;
	} },
	{ "asset:", [](string Path, string Pref) {
		if (Path.find("..") == string::npos)
			return "Assets/" + Path.substr(Pref.size());
		return string("");
	} },
	{ "file:", [](string Path, string Pref) {
		return Path.substr(Pref.size());
	} },
};

static bool UseArchives = false;

static std::mutex ArchiveMutex;
static std::map<string, Archive*> LoadedArchives;
static std::map<string, std::vector<string>> SceneArchives;

static void LoadArchive(string Name)
{
	if (LoadedArchives.contains(Name))
		return;

	Log::Note(str::Format("Loading archive: %s", Name.c_str()));
	LoadedArchives.insert({ Name, new Archive("Assets/" + Name + ".bin") });
}

static void UnloadArchive(string Name)
{
	Log::Note(str::Format("Unloading archive: %s", Name.c_str()));
	LoadedArchives.erase(Name);
}

string engine::resource::GetTextFile(string EnginePath)
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
		return kui::resource::GetStringResource(EnginePath.substr(FoundPrefix.size()));
	}
	if (FoundPrefix == "asset:" || FoundPrefix.empty())
	{
		std::ifstream in = std::ifstream(ConvertFilePath(EnginePath));
		std::stringstream instr;
		instr << in.rdbuf();
		return instr.str();
	}

	return "";
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
	string Converted = ConvertFilePath(EnginePath);

	if (UseArchives)
	{
		for (auto& i : LoadedArchives)
		{
			if (i.second->HasFile(Converted))
			{
				return true;
			}
		}
	}

	return kui::resource::FileExists(EnginePath) || std::filesystem::exists(Converted);
}

ReadOnlyBufferStream* engine::resource::GetBinaryFile(string EnginePath)
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
	if (FoundPrefix == "asset:" || FoundPrefix.empty())
	{
		std::string FilePath = ConvertFilePath(EnginePath);

		if (UseArchives)
		{
			for (auto& i : LoadedArchives)
			{
				ReadOnlyBufferStream* f = i.second->GetFile(FilePath);
				if (f)
					return f;
			}
		}

		if (std::filesystem::exists(FilePath) && !std::filesystem::is_directory(FilePath))
		{
			std::ifstream File = std::ifstream(FilePath, std::ios::binary);

			File.seekg(0, std::ios::end);
			size_t Size = File.tellg();
			File.seekg(0, std::ios::beg);

			uint8_t* Buffer = new uint8_t[Size]();

			File.read((char*)Buffer, Size);
			File.close();

			return new ReadOnlyBufferStream((const uByte*)Buffer, Size, true);
		}
	}

	return nullptr;
}

void resource::LoadSceneFiles(string ScenePath)
{
	if (!UseArchives)
	{
		return;
	}

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
		// The scenes archive is always loaded.
		if (Name == "scenes")
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
}

void resource::ScanForAssets()
{
	using std::filesystem::recursive_directory_iterator;

	UseArchives = std::filesystem::exists("Assets/archmap.bin");

	if (UseArchives)
	{
		if (LoadedArchives.empty())
		{
			LoadArchive("scenes");
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

			SceneArchives.insert({ i.Name, Scenes });
		}

		return;
	}

	LoadedAssets.clear();
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

std::map<string, string> resource::LoadedAssets;
