#pragma once
#include <Engine/Types.h>

namespace engine::cSharp
{
	void* LoadShared(string Path);
	void* GetSharedExport(void* Module, string Name);
	std::string WstrToStr(const std::wstring& wstr);
	std::wstring StrToWstr(const std::string& str);
}