#include "Platform.h"
#include <array>
#ifdef WINDOWS
#include <Windows.h>
#include <ShObjIdl_core.h>
#include <algorithm>
#include <filesystem>
#include <functiondiscoverykeys.h>
#include <map>
#include <iostream>

#undef DELETE
#include "SystemWM_SDL3.h"
#include <SDL3/SDL.h>
#include <dwmapi.h>

#pragma comment(lib, "Dwmapi.lib")

void engine::internal::platform::Init()
{
	SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
}

void engine::internal::platform::InitWindow(kui::systemWM::SysWindow* Target, int Flags)
{
	HWND hwnd = (HWND)SDL_GetPointerProperty(SDL_GetWindowProperties(Target->SDLWindow), SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);

	BOOL UseDarkMode = true;
	DwmSetWindowAttribute(
		hwnd, DWMWINDOWATTRIBUTE::DWMWA_USE_IMMERSIVE_DARK_MODE,
		&UseDarkMode, sizeof(UseDarkMode));


	if (Flags & int(kui::Window::WindowFlag::Popup))
	{
		SetWindowLong(hwnd, GWL_STYLE, WS_POPUP | WS_CAPTION | WS_THICKFRAME);
	}
}

void engine::internal::platform::Execute(string Command)
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
	wchar_t* wstr = new wchar_t[WideLength];
	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, wstr, WideLength);
	std::wstring OutStr = wstr;
	delete[] wstr;
	return OutStr;
}

void engine::internal::platform::SetConsoleColor(Log::LogColor NewColor)
{
	static std::map<Log::LogColor, WORD> WindowsColors =
	{
		std::pair(Log::LogColor::Default, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED),
		std::pair(Log::LogColor::White, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY),
		std::pair(Log::LogColor::Gray, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED),
		std::pair(Log::LogColor::Red, FOREGROUND_RED | FOREGROUND_INTENSITY),
		std::pair(Log::LogColor::Green,FOREGROUND_GREEN | FOREGROUND_INTENSITY),
		std::pair(Log::LogColor::Cyan, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY),
		std::pair(Log::LogColor::Magenta, FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_INTENSITY),
		std::pair(Log::LogColor::Blue, FOREGROUND_BLUE | FOREGROUND_INTENSITY),
		std::pair(Log::LogColor::Yellow, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY),
	};
	static HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	std::cout.flush();
	SetConsoleTextAttribute(hConsole, WindowsColors[NewColor]);
}

void engine::internal::platform::SetThreadName(string Name)
{
	SetThreadDescription(GetCurrentThread(), StrToWstr(Name).c_str());
}

engine::string engine::internal::platform::GetThreadName()
{
	PWSTR OutString;
	GetThreadDescription(GetCurrentThread(), &OutString);

	return WstrToStr(OutString);
}

engine::internal::platform::SharedLibrary* engine::internal::platform::LoadSharedLibrary(string Path)
{
	return reinterpret_cast<SharedLibrary*>(LoadLibraryW(StrToWstr(Path).c_str()));
}

void* engine::internal::platform::GetLibraryFunction(SharedLibrary* Library, string Name)
{
	return GetProcAddress(HMODULE(Library), Name.c_str());
}

std::vector<engine::string> engine::internal::platform::OpenFileDialog(std::vector<FileDialogFilter> Filters)
{
	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	if (!SUCCEEDED(hr))
	{
		return {};
	}
	IFileOpenDialog* pFileOpen = nullptr;

	// Create the FileOpenDialog object.
	hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
		IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

	std::vector<COMDLG_FILTERSPEC> Types;

	size_t FilterIndex = 0;
	size_t AllIndex = SIZE_MAX;
	for (auto& i : Filters)
	{
		std::wstring WideFilters;

		// Combine all previous filters if the name is _ALL.
		if (i.Name == "_ALL")
		{
			AllIndex = FilterIndex;
			i.Name = "All types";
			for (const FileDialogFilter& filt : Filters)
			{
				if (filt.Name == i.Name)
				{
					break;
				}

				for (const string& Type : filt.FileTypes)
				{
					i.FileTypes.push_back(Type);
				}
			}
		}

		std::ranges::for_each(i.FileTypes, [&WideFilters](const string& str)
			{
				if (str != "*")
					WideFilters += StrToWstr("*." + str) + L";";
				else
					WideFilters += L"*;";
			});

		if (WideFilters.size())
			WideFilters.pop_back();

		Types.push_back(COMDLG_FILTERSPEC{
			.pszName = SDL_wcsdup(StrToWstr(i.Name).c_str()),
			.pszSpec = SDL_wcsdup(WideFilters.c_str())
			});
		FilterIndex++;
	}

	if (!Filters.empty())
		hr = pFileOpen->SetFileTypes(UINT(Types.size()), Types.data());


	if (AllIndex != SIZE_MAX)
		pFileOpen->SetFileTypeIndex(UINT(FilterIndex - 1));

	pFileOpen->SetOptions(FOS_ALLOWMULTISELECT);

	for (auto& i : Types)
	{
		free(const_cast<wchar_t*>(i.pszName));
		free(const_cast<wchar_t*>(i.pszSpec));
	}

	if (!SUCCEEDED(hr))
	{
		pFileOpen->Release();
		CoUninitialize();
		return {};
	}

	IShellItem2* AssetsItem = nullptr;

	PROPVARIANT ItemName;

	std::wstring AssetsName = L"Project assets";

	ItemName.vt = VT_LPWSTR;
	ItemName.pwszVal = AssetsName.data();

	hr = SHCreateItemFromParsingName((std::filesystem::current_path() / "Assets").wstring().c_str(), NULL, IID_PPV_ARGS(&AssetsItem));
	if (SUCCEEDED(hr))
	{
		SHSetTemporaryPropertyForItem(AssetsItem, PKEY_ItemNameDisplay, ItemName);
		pFileOpen->AddPlace(AssetsItem, FDAP_TOP);
	}
	
	// Show the Open dialog box.
	hr = pFileOpen->Show(NULL);
	// Get the file name from the dialog box.
	if (!SUCCEEDED(hr))
	{
		pFileOpen->Release();
		CoUninitialize();
		return {};
	}
	IShellItemArray* pItems;
	hr = pFileOpen->GetResults(&pItems);
	if (SUCCEEDED(hr))
	{
		std::vector<string> Items;

		DWORD Count;
		if (!SUCCEEDED(pItems->GetCount(&Count)))
		{
			Count = 0;
		}
		for (size_t i = 0; i < Count; i++)
		{
			IShellItem* pItem;
			PWSTR pszFilePath;

			hr = pItems->GetItemAt(DWORD(i), &pItem);
			pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
			Items.push_back(WstrToStr(pszFilePath));
		}

		pFileOpen->Release();
		CoUninitialize();
		return Items;
	}

	pFileOpen->Release();
	CoUninitialize();
	return {};
}

#else
#include <map>
#include <iostream>
#include <dlfcn.h>
#include <cstring>
#include <errno.h>

void engine::internal::platform::SetConsoleColor(Log::LogColor NewColor)
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

engine::internal::platform::SharedLibrary* engine::internal::platform::LoadSharedLibrary(string Path)
{
	return reinterpret_cast<SharedLibrary*>(dlopen(Path.c_str(), RTLD_LAZY | RTLD_LOCAL));
}

void* engine::internal::platform::GetLibraryFunction(SharedLibrary* Library, string Name)
{
	return dlsym(Library, Name.c_str());
}

void engine::internal::platform::InitWindow(kui::systemWM::SysWindow* Target, int Flags)
{
}

void engine::internal::platform::Init()
{
}

void engine::internal::platform::Execute(string Command)
{
	system(Command.c_str());
}

void engine::internal::platform::SetThreadName(string Name)
{

}
engine::string engine::internal::platform::GetThreadName()
{
	return "<Unknown thread>";
}

std::vector<engine::string> engine::internal::platform::OpenFileDialog(std::vector<FileDialogFilter> Filters)
{
	// TODO: implement
	return {};
}

#endif


#if WINDOWS
static std::wstring ToWstring(std::string utf8)
{
	int WideLength = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, NULL, 0);
	wchar_t* wstr = new wchar_t[WideLength];
	MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, wstr, WideLength);
	std::wstring str = wstr;
	delete[] wstr;
	return str;
}
#else
static bool CommandExists(std::string Command)
{
	return system(("command -v " + Command + " > /dev/null").c_str()) == 0;
}
#endif

// Stolen from SystemWM_Win32 and SystemWM_Linux in KlemmUI.
// SDL also has a message box function but that one looks weird. (It doesn't even seem to use MessageBox() on Windows)
bool engine::internal::platform::ShowMessageBox(string Title, string Message, int Type)
{
	if (Type < 0 || Type > 2)
	{
		return false;
	}

#if WINDOWS
	std::array<UINT, 3> Types = { 0, MB_ICONWARNING, MB_ICONERROR };

	::MessageBoxW(NULL, ToWstring(Message).c_str(), ToWstring(Title).c_str(), Types[Type]);
	return true;
#else
	if (CommandExists("kdialog"))
	{
		std::array<const char*, 3> Types = { "msgbox", "sorry", "error" };

		Execute("/usr/bin/env kdialog --title \"" + Title + "\" --" + Types[Type] + " \"" + Message + "\"");
		return true;
	}

	if (CommandExists("zenity"))
	{
		std::array<const char*, 3> Types = { "info", "warning", "error" };

		Execute("/usr/bin/env zenity --no-markup --title \"" + Title + "\" --" + Types[Type] + " --text \"" + Message + "\"");
		return true;
	}

	// If kdialog and zenity don't exist, there's no good way of creating a message box.
	return false;
#endif

}
