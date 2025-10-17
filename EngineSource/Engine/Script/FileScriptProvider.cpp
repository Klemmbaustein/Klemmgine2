#include "FileScriptProvider.h"
#include <filesystem>
#include <fstream>
#include <sstream>
using namespace engine;
using namespace std::filesystem;

std::vector<string> engine::script::FileScriptProvider::GetFiles()
{
	if (!exists("Scripts"))
	{
		return {};
	}

	std::vector<string> Result;
	for (auto& i : recursive_directory_iterator("Scripts"))
	{
		Result.push_back(i.path().string());
	}

	return Result;
}

string engine::script::FileScriptProvider::GetFile(string Name)
{
	std::ifstream File = std::ifstream(Name);

	std::stringstream FileStream;
	FileStream << File.rdbuf();
	File.close();

	return FileStream.str();
}
