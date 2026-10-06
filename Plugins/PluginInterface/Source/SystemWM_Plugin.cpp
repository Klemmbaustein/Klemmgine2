#include "SystemWM_Plugin.h"
#include <SystemWM.h>

using namespace kui;
using namespace kui::systemWM;
kui::systemWM::SysWindow* kui::systemWM::NewWindow(
	Window* Parent, Vec2ui Size, Vec2ui Pos, std::string Title, Window::WindowFlag Flags)
{
	return nullptr;
}

void kui::systemWM::DestroyWindow(SysWindow* Target)
{
}

void kui::systemWM::SwapWindow(SysWindow* Target)
{
}

void kui::systemWM::WaitFrame(SysWindow* Target, float RemainingTime)
{
}

void* kui::systemWM::GetPlatformHandle(SysWindow* Target)
{
	return nullptr;
}

void kui::systemWM::ActivateContext(SysWindow* Target)
{
}

kui::Vec2ui kui::systemWM::GetWindowSize(SysWindow* Target)
{
	return Vec2ui();
}

void kui::systemWM::SetWindowIcon(SysWindow* Target, uint8_t* Bytes, size_t Width, size_t Height)
{
}

void kui::systemWM::UpdateWindow(SysWindow* Target)
{
}

bool kui::systemWM::WindowHasFocus(SysWindow* Target)
{
	return false;
}

bool kui::systemWM::WindowHasMouseFocus(SysWindow* Target)
{
	return false;
}

kui::Vec2i kui::systemWM::GetCursorPosition(SysWindow* Target)
{
	return Vec2ui();
}

kui::Vec2ui kui::systemWM::GetScreenSize()
{
	return Vec2ui();
}

std::string kui::systemWM::GetTextInput(SysWindow* Target)
{
	return "";
}

uint32_t kui::systemWM::GetDesiredRefreshRate(SysWindow* From)
{
	return 60;
}

void kui::systemWM::SetWindowCursor(SysWindow* Target, Window::Cursor NewCursor)
{
}

float kui::systemWM::GetDPIScale(SysWindow* Target)
{
	return 1;
}

void kui::systemWM::SetClipboardText(std::string NewText)
{
}

std::string kui::systemWM::GetClipboardText()
{
	return "";
}

bool kui::systemWM::IsLMBDown()
{
	return false;
}

bool kui::systemWM::IsRMBDown()
{
	return false;
}

void kui::systemWM::SetWindowSize(SysWindow* Target, Vec2ui Size)
{
}

void kui::systemWM::SetWindowPosition(SysWindow* Target, Vec2ui NewPosition)
{
}

void kui::systemWM::SetTitle(SysWindow* Target, std::string Text)
{
}

bool kui::systemWM::IsWindowFullScreen(SysWindow* Target)
{
	return false;
}

void kui::systemWM::SetWindowMinSize(SysWindow* Target, Vec2ui MinSize)
{
}

void kui::systemWM::SetWindowMaxSize(SysWindow* Target, Vec2ui MaxSize)
{
}

void kui::systemWM::RestoreWindow(SysWindow* Target)
{
}

void kui::systemWM::MinimizeWindow(SysWindow* Target)
{
}

void kui::systemWM::MaximizeWindow(SysWindow* Target)
{
}

void kui::systemWM::UpdateWindowFlags(SysWindow* Target, Window::WindowFlag NewFlags)
{
}

bool kui::systemWM::IsWindowMinimized(SysWindow* Target)
{
	return false;
}

void kui::systemWM::HideWindow(SysWindow* Target)
{
}

void kui::systemWM::MessageBox(std::string Text, std::string Title, int Type)
{
}

bool kui::systemWM::YesNoBox(std::string, std::string)
{
	return false;
}

std::string kui::systemWM::SelectFileDialog(bool PickFolders)
{
	return "";
}
