#pragma once
#include <Engine/Types.h>
#include <Engine/Log.h>
#include <vector>

namespace kui::systemWM
{
	class SysWindow;
}

namespace engine::internal::platform
{
	void Init();
	void InitWindow(kui::systemWM::SysWindow* Target, int Flags);

	void Execute(string Command);
	

	void ShowMessageBox(string Title, string Message, int Type);

	struct FileDialogFilter
	{
		string Name;
		std::vector<string> FileTypes;
	};

	void SetConsoleColor(Log::LogColor NewColor);
	
	std::vector<string> OpenFileDialog(std::vector<FileDialogFilter> Filters);
}