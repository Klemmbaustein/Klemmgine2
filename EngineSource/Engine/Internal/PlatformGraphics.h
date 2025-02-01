#pragma once
#include <Core/Types.h>
#include <Core/Log.h>
#include <vector>

namespace kui::systemWM
{
	class SysWindow;
}

namespace engine::internal::platform
{
	struct FileDialogFilter
	{
		string Name;
		std::vector<string> FileTypes;
	};

	void Init();
	void InitWindow(kui::systemWM::SysWindow* Target, int Flags);
	bool ShowMessageBox(string Title, string Message, int Type);
	std::vector<string> OpenFileDialog(std::vector<FileDialogFilter> Filters);
}