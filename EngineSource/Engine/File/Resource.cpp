#include "Resource.h"
#include <map>
#include <functional>
#include <fstream>
#include <sstream>
#include <Engine/Log.h>
#include <filesystem>
#include <kui/Resource.h>

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
			Log::Warn(str::Format("Unexpected file: prefix while attempting to load asset: %s", EnginePath.c_str()));
			continue;
		}

		FullPath = Convert(EnginePath, Prefix);
	}

	auto Found = LoadedAssets.find(EnginePath);

	if (Found != LoadedAssets.end())
	{
		return Found->second;
	}
	return "Assets/" + EnginePath;
}

bool engine::resource::FileExists(string EnginePath)
{
	return kui::resource::FileExists(EnginePath) || std::filesystem::exists(ConvertFilePath(EnginePath));
}

BinaryFile engine::resource::GetBinaryFile(string EnginePath)
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

		return BinaryFile{
			.DataPtr = res.Data,
			.DataSize = res.FileSize,
			.CanFree = false,
		};
	}
	if (FoundPrefix == "asset:" || FoundPrefix.empty())
	{
		std::string FilePath = ConvertFilePath(EnginePath);
		if (std::filesystem::exists(FilePath) && !std::filesystem::is_directory(FilePath))
		{
			std::ifstream File = std::ifstream(FilePath, std::ios::binary);

			File.seekg(0, std::ios::end);
			size_t Size = File.tellg();
			File.seekg(0, std::ios::beg);

			uint8_t* Buffer = new uint8_t[Size]();

			File.read((char*)Buffer, Size);
			File.close();

			return BinaryFile{
				.DataPtr = Buffer,
				.DataSize = Size,
			};
		}
	}

	return BinaryFile();
}

void engine::resource::FreeBinaryFile(BinaryFile& Target)
{
	if (!Target.CanFree)
		return;

	delete[] Target.DataPtr;
	Target.DataPtr = nullptr;
	Target.DataSize = 0;
	Target.CanFree = false;
}

void engine::resource::ScanForAssets()
{
	for (const auto& i : std::filesystem::recursive_directory_iterator("Assets/"))
	{
		if (i.is_regular_file())
			LoadedAssets.insert({ i.path().filename().string(), str::ReplaceChar(i.path().string(), '\\', '/') });
	}
}

std::map<engine::string, engine::string> resource::LoadedAssets;
