#pragma once
#include <Engine/Types.h>
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

	string OpenFileDialog(std::vector<FileDialogFilter> Filters);
}