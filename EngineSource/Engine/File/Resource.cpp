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

static std::set<ResourceSource*> Sources;

string engine::resource::GetTextFile(string EnginePath)
{
	auto f = GetBinaryFile(EnginePath);

	if (!f)
	{
		return "";
	}

	string FileContent = string((char*)f->GetData(), f->GetSize());

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
	string Converted = ConvertFilePath(EnginePath);

	for (auto& i : Sources)
	{
		if (i->FileExists(EnginePath))
		{
			return true;
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

		for (auto& i : Sources)
		{
			if (i->FileExists(EnginePath))
			{
				return i->GetFile(EnginePath);
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
	for (auto& i : Sources)
	{
		i->LoadSceneFiles(ScenePath);
	}
}

void resource::ScanForAssets()
{
	using std::filesystem::recursive_directory_iterator;

	LoadedAssets.clear();

	for (auto& i : Sources)
	{
		auto SourceAssets = i->GetFiles();

		for (auto& asset : SourceAssets)
		{
			LoadedAssets.insert(asset);
		}
	}
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

void resource::AddResourceSource(ResourceSource* Source)
{
	Sources.insert(Source);
}

void resource::RemoveResourceSource(ResourceSource* Source)
{
	Sources.erase(Source);
}

std::map<string, string> resource::LoadedAssets;
