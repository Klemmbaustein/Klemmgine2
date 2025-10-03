#pragma once
#include <Core/Types.h>

namespace engine::file
{
	string FileNameWithoutExt(string FromPath);
	string FileName(string FromPath);
	string FilePath(string PathWithName);
	string Extension(string FromPath);
}