#include "FileUtil.h"

using namespace engine;

string engine::file::FileNameWithoutExt(string FromPath)
{
	string Name = FileName(FromPath);

	size_t Dot = Name.find_last_of('.');

	if (Dot == string::npos)
	{
		return Name;
	}

	return Name.substr(0, Dot);
}

string engine::file::FileName(string FromPath)
{
#ifdef WINDOWS
	return FromPath.substr(FromPath.find_last_of("\\/") + 1);
#else
	return FromPath.substr(FromPath.find_last_of("/") + 1);
#endif
}

string engine::file::FilePath(string PathWithName)
{
#ifdef WINDOWS
	size_t LastSlash = PathWithName.find_last_of("\\/");
#else
	size_t LastSlash = PathWithName.find_last_of("/");
#endif
	if (LastSlash == std::string::npos)
	{
		return "";
	}
	return PathWithName.substr(0, LastSlash);
}

string engine::file::Extension(string FromPath)
{
	FromPath = FileName(FromPath);
	size_t Found = FromPath.find_last_of('.');

	if (Found == string::npos)
	{
		return "";
	}
	return FromPath.substr(Found + 1);
}
