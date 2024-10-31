#ifndef SERVER
#include "SystemWM_SDL3.h"
#include <SystemWM.h>
#include <kui/App.h>
#include <iostream>
#include <Engine/Engine.h>
#include <Engine/Subsystem/InputSubsystem.h>

static std::mutex EventsMutex;

std::vector<kui::systemWM::SysWindow*> ActiveWindows;

kui::systemWM::SysWindow* kui::systemWM::NewWindow(
	Window* Parent, Vec2ui Size, Vec2ui Pos, std::string Title, Window::WindowFlag Flags)
{
	std::lock_guard g{ EventsMutex };
	SysWindow* OutWindow = new SysWindow();
	OutWindow->Parent = Parent;
	OutWindow->SDLWindow = SDL_CreateWindow(Title.c_str(), int(Size.X), int(Size.Y), SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

	OutWindow->GLContext = SDL_GL_CreateContext(OutWindow->SDLWindow);

	ActiveWindows.push_back(OutWindow);

	return OutWindow;
}

void kui::systemWM::DestroyWindow(SysWindow* Target)
{
	SDL_GL_DestroyContext(Target->GLContext);
	SDL_DestroyWindow(Target->SDLWindow);
	delete Target;
}

void kui::systemWM::SwapWindow(SysWindow* Target)
{
	// Don't swap, the engine does that elsewhere.
}

void kui::systemWM::WaitFrame(SysWindow* Target, float RemainingTime)
{
}

void kui::systemWM::ActivateContext(SysWindow* Target)
{
	SDL_GL_MakeCurrent(Target->SDLWindow, Target->GLContext);
}

kui::Vec2ui kui::systemWM::GetWindowSize(SysWindow* Target)
{
	int w, h;
	SDL_GetWindowSize(Target->SDLWindow, &w, &h);
	return Vec2ui(w, h);
}

void kui::systemWM::SetWindowIcon(SysWindow* Target, uint8_t* Bytes, size_t Width, size_t Height)
{
}

void kui::systemWM::UpdateWindow(SysWindow* Target)
{
	{
		std::lock_guard g{ EventsMutex };
		
		SDL_Event ev;
		SDL_Window* LastWindow = nullptr;
		SDL_WindowID LastID = 0;
		while (SDL_PollEvent(&ev))
		{
			for (auto& i : ActiveWindows)
			{
				if (SDL_GetWindowID(i->SDLWindow) == ev.window.windowID)
				{
					i->Events.push_back(ev);
				}
			}
		}

	}
	Target->UpdateEvents();
}

bool kui::systemWM::WindowHasFocus(SysWindow* Target)
{
	return SDL_GetKeyboardFocus() == Target->SDLWindow;
}

kui::Vec2i kui::systemWM::GetCursorPosition(SysWindow* Target)
{
	float x, y;
	SDL_GetGlobalMouseState(&x, &y);
	int winX, winY;
	SDL_GetWindowPosition(Target->SDLWindow, &winX, &winY);

	return { int(x) - winX, int(y) - winY };
}

kui::Vec2ui kui::systemWM::GetScreenSize()
{
	SDL_Rect out;
	if (!SDL_GetDisplayBounds(SDL_GetPrimaryDisplay(), &out))
	{
		app::error::Error(SDL_GetError());
		return Vec2ui(1920, 1080);
	}
	return Vec2ui(out.w, out.h);
}

std::string kui::systemWM::GetTextInput(SysWindow* Target)
{
	return "";
}

uint32_t kui::systemWM::GetDesiredRefreshRate(SysWindow* From)
{
	const SDL_DisplayMode* Mode = SDL_GetCurrentDisplayMode(SDL_GetDisplayForWindow(From->SDLWindow));

	if (!Mode)
	{
		kui::app::error::Error(SDL_GetError());
		return 60;
	}

	uint32_t RefreshRate = uint32_t(Mode->refresh_rate);

	if (RefreshRate == 0)
	{
		RefreshRate = 60;
	}

	return RefreshRate;
}

void kui::systemWM::SetWindowCursor(SysWindow* Target, Window::Cursor NewCursor)
{
}

float kui::systemWM::GetDPIScale(SysWindow* Target)
{
	float Density = SDL_GetWindowPixelDensity(Target->SDLWindow);

	if (Density == 0)
	{
		Density = 1;
	}

	return Density;
}

void kui::systemWM::SetClipboardText(std::string NewText)
{
	if (!SDL_SetClipboardText(NewText.c_str()))
	{
		kui::app::error::Error(SDL_GetError());
	}
}

std::string kui::systemWM::GetClipboardText()
{
	char* Clipboard = SDL_GetClipboardText();
	std::string OutString = Clipboard;
	SDL_free(Clipboard);
	return OutString;
}

bool kui::systemWM::IsLMBDown()
{
	SDL_MouseButtonFlags State = SDL_GetGlobalMouseState(nullptr, nullptr);

	return State & SDL_BUTTON_LEFT;
}

bool kui::systemWM::IsRMBDown()
{
	SDL_MouseButtonFlags State = SDL_GetGlobalMouseState(nullptr, nullptr);

	return State & SDL_BUTTON_RIGHT;
}

void kui::systemWM::SetWindowSize(SysWindow* Target, Vec2ui Size)
{
	SDL_SetWindowSize(Target->SDLWindow, int(Size.X), int(Size.Y));
}

void kui::systemWM::SetWindowPosition(SysWindow* Target, Vec2ui NewPosition)
{
	SDL_SetWindowPosition(Target->SDLWindow, int(NewPosition.X), int(NewPosition.Y));
}

void kui::systemWM::SetTitle(SysWindow* Target, std::string Text)
{
	SDL_SetWindowTitle(Target->SDLWindow, Text.c_str());
}

bool kui::systemWM::IsWindowFullScreen(SysWindow* Target)
{
	return SDL_GetWindowFlags(Target->SDLWindow) & SDL_WINDOW_MAXIMIZED;
}

void kui::systemWM::SetWindowMinSize(SysWindow* Target, Vec2ui MinSize)
{
	SDL_SetWindowMinimumSize(Target->SDLWindow, int(MinSize.X), int(MinSize.Y));
}

void kui::systemWM::SetWindowMaxSize(SysWindow* Target, Vec2ui MaxSize)
{
	SDL_SetWindowMaximumSize(Target->SDLWindow, int(MaxSize.X), int(MaxSize.Y));
}

void kui::systemWM::RestoreWindow(SysWindow* Target)
{
	SDL_RestoreWindow(Target->SDLWindow);
}

void kui::systemWM::MinimizeWindow(SysWindow* Target)
{
	SDL_MinimizeWindow(Target->SDLWindow);
}

void kui::systemWM::MaximizeWindow(SysWindow* Target)
{
	SDL_MaximizeWindow(Target->SDLWindow);
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

void kui::systemWM::SysWindow::UpdateEvents()
{
	using namespace engine;
	using namespace engine::subsystem;

	std::lock_guard g{EventsMutex};

	std::vector EventsCopy = Events;
	Events.clear();

	InputSubsystem* InputSys = Engine::GetSubsystem<InputSubsystem>();
	input::MouseMovement = 0;

	for (auto& ev : EventsCopy)
	{
		switch (ev.type)
		{
		case SDL_EVENT_WINDOW_RESIZED:
			Parent->OnResized();
			break;
		case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
			Parent->Close();
			break;
		case SDL_EVENT_KEY_DOWN:
			InputSys->SetKeyDown(input::Key(ev.key.key), true);
			break;
		case SDL_EVENT_KEY_UP:
			InputSys->SetKeyDown(input::Key(ev.key.key), false);
			break;
		case SDL_EVENT_MOUSE_MOTION:
			if (WindowHasFocus(this) && !input::ShowMouseCursor)
				input::MouseMovement = input::MouseMovement + Vector2(ev.motion.xrel, ev.motion.yrel) * Vector2(0.001f, 0.001f);
			break;
		default:
			break;
		}
	}
}
#endif