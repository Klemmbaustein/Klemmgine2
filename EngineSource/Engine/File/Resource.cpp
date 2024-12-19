#include "Resource.h"
#include <map>
#include <functional>
#include <fstream>
#include <sstream>
#include <Engine/Log.h>
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

	return FullPath;
}

BinaryFile engine::resource::GetBinaryFile(string EnginePath)
{
	return BinaryFile();
}

void engine::resource::FreeBinaryFile(BinaryFile Target)
{
}
