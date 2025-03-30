#include "Platform.h"
#include <array>
#ifdef WINDOWS
#include <Windows.h>
#include <map>
#include <iostream>

#undef DELETE
void engine::platform::Execute(string Command)
{
	STARTUPINFO Startup;
	ZeroMemory(&Startup, sizeof(Startup));
	Startup.cb = sizeof(Startup);

	PROCESS_INFORMATION ProcInfo;
	ZeroMemory(&ProcInfo, sizeof(ProcInfo));

	if (!CreateProcess(NULL, Command.data(), NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &Startup, &ProcInfo))
	{
		return;
	}
	WaitForSingleObject(ProcInfo.hProcess, INFINITE);
	CloseHandle(ProcInfo.hProcess);
	CloseHandle(ProcInfo.hThread);
}

void engine::platform::Open(string File)
{
	ShellExecute(NULL, "open", File.c_str(), NULL, NULL, FALSE);
}

static std::string WstrToStr(const std::wstring& wstr)
{
	std::string strTo;
	char* szTo = new char[wstr.length() + 1];
	szTo[wstr.size()] = '\0';
	WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, szTo, (int)wstr.length(), NULL, NULL);
	strTo = szTo;
	delete[] szTo;
	return strTo;
}

static std::wstring StrToWstr(const std::string& str)
{
	int WideLength = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
	wchar_t* wstr = new wchar_t[WideLength + 1];
	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, wstr, WideLength);
	wstr[WideLength] = 0;
	std::wstring OutStr = wstr;
	delete[] wstr;
	return OutStr;
}

void engine::platform::SetConsoleColor(Log::LogColor NewColor)
{
	static std::map<Log::LogColor, WORD> WindowsColors =
	{
		{ Log::LogColor::Default, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED },
		{ Log::LogColor::White, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY },
		{ Log::LogColor::Gray, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED },
		{ Log::LogColor::Red, FOREGROUND_RED | FOREGROUND_INTENSITY },
		{ Log::LogColor::Green, FOREGROUND_GREEN | FOREGROUND_INTENSITY },
		{ Log::LogColor::Cyan, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY },
		{ Log::LogColor::Magenta, FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_INTENSITY },
		{ Log::LogColor::Blue, FOREGROUND_BLUE | FOREGROUND_INTENSITY },
		{ Log::LogColor::Yellow, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY },
	};
	static HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	std::cout.flush();
	SetConsoleTextAttribute(hConsole, WindowsColors[NewColor]);
}

void engine::platform::SetThreadName(string Name)
{
	SetThreadDescription(GetCurrentThread(), StrToWstr(Name).c_str());
}

engine::string engine::platform::GetThreadName()
{
	PWSTR OutString;
	GetThreadDescription(GetCurrentThread(), &OutString);

	return WstrToStr(OutString);
}

engine::platform::SharedLibrary* engine::platform::LoadSharedLibrary(string Path)
{
	return reinterpret_cast<SharedLibrary*>(LoadLibraryW(StrToWstr(Path).c_str()));
}

void* engine::platform::GetLibraryFunction(SharedLibrary* Library, string Name)
{
	return GetProcAddress(HMODULE(Library), Name.c_str());
}

void engine::platform::UnloadSharedLibrary(SharedLibrary* Library)
{
	FreeLibrary(reinterpret_cast<HMODULE>(Library));
}

#else
#include <map>
#include <iostream>
#include <dlfcn.h>
#include <cstring>
#include <errno.h>
#include <thread>

void engine::platform::SetConsoleColor(Log::LogColor NewColor)
{
	static std::map<Log::LogColor, const char*> ColorCodes =
	{
		std::pair(Log::LogColor::Default,"\x1B[0m"),
		std::pair(Log::LogColor::White, "\x1B[37m"),
		std::pair(Log::LogColor::Gray, "\x1B[0m"),
		std::pair(Log::LogColor::Cyan, "\x1B[36m"),
		std::pair(Log::LogColor::Magenta, "\x1B[35m"),
		std::pair(Log::LogColor::Red, "\x1B[31m"),
		std::pair(Log::LogColor::Green, "\x1B[32m"),
		std::pair(Log::LogColor::Blue, "\x1B[34m"),
		std::pair(Log::LogColor::Yellow, "\x1B[33m"),
	};

	printf("%s", ColorCodes[NewColor]);
}

engine::platform::SharedLibrary* engine::platform::LoadSharedLibrary(string Path)
{
	return reinterpret_cast<SharedLibrary*>(dlopen(Path.c_str(), RTLD_LAZY | RTLD_LOCAL));
}

void* engine::platform::GetLibraryFunction(SharedLibrary* Library, string Name)
{
	return dlsym(Library, Name.c_str());
}

void engine::platform::UnloadSharedLibrary(SharedLibrary* Library)
{
	dlclose(Library);
}

void engine::platform::Execute(string Command)
{
	system(Command.c_str());
}

void engine::platform::Open(string File)
{
	std::thread([File]() {system(("xdg-open " + File).c_str()); }).detach();
}

void engine::platform::SetThreadName(string Name)
{

}
engine::string engine::platform::GetThreadName()
{
	return "<Unknown thread>";
}

#endif
