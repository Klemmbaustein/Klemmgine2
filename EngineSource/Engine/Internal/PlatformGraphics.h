#pragma once
#include <Core/Types.h>
#include <Core/Log.h>
#include <vector>
#include <kui/Window.h>

namespace kui::systemWM
{
	class SysWindow;
}

namespace engine::platform
{
	struct FileDialogFilter
	{
		string Name;
		std::vector<string> FileTypes;
	};

	void Init();
	void InitWindow(kui::systemWM::SysWindow* Target, int Flags);
	bool ShowMessageBox(string Title, string Message, int Type);

	void SetWindowTheming(kui::Vec3f Color, kui::Vec3f TextColor,
		kui::Vec3f BorderColor, bool RoundCorners, kui::Window* Window);

	std::vector<string> OpenFileDialog(std::vector<FileDialogFilter> Filters);
}