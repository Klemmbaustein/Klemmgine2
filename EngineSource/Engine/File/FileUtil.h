#pragma once
#include <Engine/Types.h>

namespace engine::file
{
	string FileNameWithoutExt(string FromPath);
	string FileName(string FromPath);
	string FilePath(string PathWithName);
}