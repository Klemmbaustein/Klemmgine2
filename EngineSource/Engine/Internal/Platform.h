#pragma once
#include <Engine/Types.h>
#include <Engine/Log.h>
#include <vector>

namespace engine::internal::platform
{
	void Init();

	void Execute(string Command);
	
	struct FileDialogFilter
	{
		string Name;
		std::vector<string> FileTypes;
	};

	void SetConsoleColor(Log::LogColor NewColor);
	
	string OpenFileDialog(std::vector<FileDialogFilter> Filters);
}