#include "Platform.h"
#ifdef WINDOWS
#include <Windows.h>
#include <ShObjIdl_core.h>
#include <algorithm>
#include <filesystem>
#include <functiondiscoverykeys.h>
#include <map>
#include <iostream>

void engine::internal::platform::Init()
{
	SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
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
		std::pair(Log::LogColor::Blue, FOREGROUND_BLUE | FOREGROUND_INTENSITY),
		std::pair(Log::LogColor::Yellow, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY),
	};
	static HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	std::cout.flush();
	SetConsoleTextAttribute(hConsole, WindowsColors[NewColor]);
}

engine::string engine::internal::platform::OpenFileDialog(std::vector<FileDialogFilter> Filters)
{
	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	if (!SUCCEEDED(hr))
	{
		return "";
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
			.pszName = wcsdup(StrToWstr(i.Name).c_str()),
			.pszSpec = wcsdup(WideFilters.c_str())
			});
		FilterIndex++;
	}

	if (!Filters.empty())
		hr = pFileOpen->SetFileTypes(UINT(Types.size()), Types.data());


	if (AllIndex != SIZE_MAX)
		pFileOpen->SetFileTypeIndex(UINT(FilterIndex - 1));

	for (auto& i : Types)
	{
		free(const_cast<wchar_t*>(i.pszName));
		free(const_cast<wchar_t*>(i.pszSpec));
	}

	if (!SUCCEEDED(hr))
	{
		return "";
	}

	IShellItem2* AssetsItem = nullptr;

	PROPVARIANT ItemName;

	std::wstring AssetsName = L"Project assets";

	ItemName.vt = VT_LPWSTR;
	ItemName.pwszVal = AssetsName.data();

	pFileOpen->SetOkButtonLabel(L"Import");

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
		return "";
	}
	IShellItem* pItem;
	hr = pFileOpen->GetResult(&pItem);
	if (SUCCEEDED(hr))
	{
		PWSTR pszFilePath;
		hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
		// Display the file name to the user.
		if (SUCCEEDED(hr))
		{
			return WstrToStr(pszFilePath);
		}
		pItem->Release();
	}

	pFileOpen->Release();
	CoUninitialize();
	return "";
}

#else
#include <map>
#include <iostream>

void engine::internal::platform::SetConsoleColor(Log::LogColor NewColor)
{
	static std::map<Log::LogColor, const char*> ColorCodes =
	{
		std::pair(Log::LogColor::Default, "39"),
		std::pair(Log::LogColor::White, "97"),
		std::pair(Log::LogColor::Gray, "39"),
		std::pair(Log::LogColor::Red, "91"),
		std::pair(Log::LogColor::Green, "92"),
		std::pair(Log::LogColor::Blue, "94"),
		std::pair(Log::LogColor::Yellow, "93"),
	};

	std::cout << "\033[" << ColorCodes[NewColor] << "m";
}

void engine::internal::platform::Init()
{
}

void engine::internal::platform::Execute(string Command)
{
	system(Command.c_str());
}

engine::string engine::internal::platform::OpenFileDialog(std::vector<FileDialogFilter> Filters)
{
	// TODO: implement
	return "";
}

#endif