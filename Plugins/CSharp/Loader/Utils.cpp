#include "Utils.h"

#ifdef _WIN32
#include <Windows.h>

std::string engine::cSharp::WstrToStr(const std::wstring& wstr)
{
	std::string strTo;
	char* szTo = new char[wstr.length() + 1];
	szTo[wstr.size()] = '\0';
	WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, szTo, (int)wstr.length(), NULL, NULL);
	strTo = szTo;
	delete[] szTo;
	return strTo;
}

std::wstring engine::cSharp::StrToWstr(const std::string& str)
{
	int WideLength = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
	wchar_t* wstr = new wchar_t[WideLength];
	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, wstr, WideLength);
	std::wstring OutStr = wstr;
	delete[] wstr;
	return OutStr;
}

void* engine::cSharp::LoadShared(string Path)
{
	HMODULE h = ::LoadLibraryW(StrToWstr(Path).c_str());
	//ENGINE_ASSERT(h != nullptr, "Could not load library");
	return (void*)h;
}
void* engine::cSharp::GetSharedExport(void* Module, string Path)
{
	void* f = (void*)::GetProcAddress((HMODULE)Module, Path.c_str());
	//ENGINE_ASSERT(f != nullptr, "Could not export library: " + std::string(name));
	return f;
}

#else
#include <dlfcn.h>
#include <limits.h>

void* engine::cSharp::LoadShared(string Path)
{
	void* h = dlopen(Path.c_str(), RTLD_LAZY | RTLD_LOCAL);
	//ENGINE_ASSERT(h != nullptr, "Could not load library.");
	return h;
}
void* engine::cSharp::GetSharedExport(void* Module, string Path)
{
	void* f = dlsym(Module, Path.c_str());
	//ENGINE_ASSERT(f != nullptr, "Could not export library");
	return f;
}

#endif
