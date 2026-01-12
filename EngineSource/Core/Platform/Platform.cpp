#include "Platform.h"
using namespace engine;

#ifdef WINDOWS
#include <Windows.h>
#include <map>
#include <filesystem>
#include <iostream>
#include <Lmcons.h>

#undef DELETE
void engine::platform::Execute(string Command)
{
	STARTUPINFO Startup;
	ZeroMemory(&Startup, sizeof(Startup));
	Startup.cb = sizeof(Startup);

	PROCESS_INFORMATION ProcInfo;
	ZeroMemory(&ProcInfo, sizeof(ProcInfo));

	if (!CreateProcess(NULL, Command.data(), NULL, NULL, FALSE, CREATE_NO_WINDOW,
		NULL, NULL, &Startup, &ProcInfo))
	{
		return;
	}
	WaitForSingleObject(ProcInfo.hProcess, INFINITE);
	CloseHandle(ProcInfo.hProcess);
	CloseHandle(ProcInfo.hThread);
}

void engine::platform::Open(string File)
{
	ShellExecuteW(NULL, L"open", StrToWstr(File).c_str(), NULL, NULL, FALSE);
}

engine::string engine::platform::GetLastErrorString()
{
	DWORD ErrorMessageID = GetLastError();
	if (ErrorMessageID == 0)
	{
		return "No error";
	}

	LPWSTR MessageBuffer = nullptr;
	size_t Size = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER
		| FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, ErrorMessageID, MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT),
		(LPWSTR)&MessageBuffer, 0, NULL);

	std::wstring Message = std::wstring(MessageBuffer, Size);

	LocalFree(MessageBuffer);

	return WstrToStr(Message);
}

std::string engine::platform::WstrToStr(const std::wstring& wstr)
{
	std::string strTo;
	char* szTo = new char[wstr.length() + 1];
	szTo[wstr.size()] = '\0';
	WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, szTo, (int)wstr.length(), NULL, NULL);
	strTo = szTo;
	delete[] szTo;
	return strTo;
}

std::wstring engine::platform::StrToWstr(const std::string& str)
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
	std::wstring WidePath = StrToWstr(Path);
	if (std::filesystem::exists(WidePath))
		WidePath = std::filesystem::canonical(WidePath);

	return reinterpret_cast<SharedLibrary*>(LoadLibraryExW(WidePath.c_str(),
		NULL, LOAD_LIBRARY_SEARCH_DEFAULT_DIRS | LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR));
}

engine::string engine::platform::GetSystemUserName()
{
	wchar_t UserName[UNLEN + 1];
	DWORD Length = sizeof(UserName) / sizeof(wchar_t);

	GetUserNameW(UserName, &Length);

	return WstrToStr(std::wstring(UserName, Length));
}

void* engine::platform::GetLibraryFunction(SharedLibrary* Library, string Name)
{
	return GetProcAddress(HMODULE(Library), Name.c_str());
}

void engine::platform::UnloadSharedLibrary(SharedLibrary* Library)
{
	FreeLibrary(reinterpret_cast<HMODULE>(Library));
}

engine::string engine::platform::GetExecutablePath()
{
	wchar_t FileName[MAX_PATH];
	GetModuleFileNameW(NULL, FileName, MAX_PATH);
	return WstrToStr(FileName);
}

#else
#include <map>
#include <iostream>
#include <dlfcn.h>
#include <cstring>
#include <errno.h>
#include <filesystem>
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
	if (std::filesystem::exists(Path))
		Path = std::filesystem::canonical(Path);

	SharedLibrary* Loaded = reinterpret_cast<SharedLibrary*>(dlopen(Path.c_str(), RTLD_LAZY | RTLD_LOCAL));

	if (!Loaded)
	{
		Log::Error(strerror(errno));
	}
	return Loaded;

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

engine::string engine::platform::GetSystemUserName()
{
	return getenv("USER");
}

engine::string engine::platform::GetThreadName()
{
	char ThreadBuffer[8000];
	pthread_getname_np(pthread_self(), ThreadBuffer, sizeof(ThreadBuffer));
	return ThreadBuffer;
}
engine::string engine::platform::GetExecutablePath()
{
	FILE* SelfLinkCommand = popen(("readlink /proc/" + std::to_string(getpid()) + "/exe").c_str(), "r");

	if (SelfLinkCommand == nullptr)
	{
		std::cout << "failed to call popen() to read process name." << std::endl;
		return "";
	}

	while (!feof(SelfLinkCommand))
	{
		char ExecutableBuffer[8000];
		size_t Read = fread(ExecutableBuffer, 1, sizeof(ExecutableBuffer) - 1, SelfLinkCommand);
		ExecutableBuffer[Read] = 0;
		if (Read > 0 && ExecutableBuffer[Read - 1] == '\n')
		{
			ExecutableBuffer[Read - 1] = 0;
		}
		pclose(SelfLinkCommand);
		return ExecutableBuffer;
	}
	pclose(SelfLinkCommand);
	return "";
}

#endif

#if WINDOWS
#define popen _popen
#define pclose _pclose
#endif

string engine::platform::GetCommandOutput(string Command)
{
	FILE* CommandPipe = popen(Command.c_str(), "r");

	string Output;

	if (CommandPipe == nullptr)
	{
		return Output;
	}

	while (!feof(CommandPipe))
	{
		char NewDataBuffer[8000];
		size_t Read = fread(NewDataBuffer, 1, sizeof(NewDataBuffer) - 1, CommandPipe);
		NewDataBuffer[Read] = 0;
		Output.append(NewDataBuffer);
	}
	pclose(CommandPipe);
	return Output;
}
