#include "PlatformGraphics.h"
#include <Core/Platform/Platform.h>
#include "SystemWM_SDL3.h"
#include <SDL3/SDL.h>
#include <algorithm>
#include <filesystem>
#pragma comment(lib, "Dwmapi.lib")

#if WINDOWS
#include <ShObjIdl_core.h>
#include <functiondiscoverykeys.h>
#include <dwmapi.h>

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

void engine::platform::Init()
{
#ifdef NTDDI_WIN10
	SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
#endif
}

void engine::platform::InitWindow(kui::systemWM::SysWindow* Target, int Flags)
{
	HWND hwnd = (HWND)SDL_GetPointerProperty(SDL_GetWindowProperties(Target->SDLWindow), SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);

	BOOL UseDarkMode = true;
	DwmSetWindowAttribute(
		hwnd, DWMWINDOWATTRIBUTE::DWMWA_USE_IMMERSIVE_DARK_MODE,
		&UseDarkMode, sizeof(UseDarkMode));


	if (Flags & int(kui::Window::WindowFlag::Popup))
	{
		SetWindowLong(hwnd, GWL_STYLE, WS_POPUP | WS_CAPTION);
	}
}

std::vector<engine::string> engine::platform::OpenFileDialog(std::vector<FileDialogFilter> Filters)
{
	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	if (!SUCCEEDED(hr))
	{
		return {};
	}

	IFileOpenDialog* pFileOpen = nullptr;

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

			if (!pszFilePath)
				continue;

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

static bool CommandExists(std::string Command)
{
	return system(("command -v " + Command + " > /dev/null").c_str()) == 0;
}

void engine::platform::InitWindow(kui::systemWM::SysWindow* Target, int Flags)
{
}

void engine::platform::Init()
{
}

std::vector<engine::string> engine::platform::OpenFileDialog(std::vector<FileDialogFilter> Filters)
{
	// TODO: implement
	return {};
}

#endif

// Stolen from SystemWM_Win32 and SystemWM_Linux in KlemmUI.
// SDL also has a message box function but that one looks weird. (It doesn't even seem to use MessageBox() on Windows)
bool engine::platform::ShowMessageBox(string Title, string Message, int Type)
{
	if (Type < 0 || Type > 2)
	{
		return false;
	}

#if WINDOWS
	std::array<UINT, 3> Types = { 0, MB_ICONWARNING, MB_ICONERROR };

	::MessageBoxW(NULL, StrToWstr(Message).c_str(), StrToWstr(Title).c_str(), Types[Type]);
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
