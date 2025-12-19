#ifndef SERVER
#include "SystemWM_SDL3.h"
#include <SystemWM.h>
#include <kui/App.h>
#include <thread>
#include <Engine/Engine.h>
#include <Engine/Graphics/VideoSubsystem.h>
#include <Engine/Subsystem/InputSubsystem.h>
#include "PlatformGraphics.h"
#include <Engine/MainThread.h>
#include "WMOptions.h"

#if WINDOWS
#pragma comment(linker, "\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

static std::mutex WindowCreateMutex;

static std::vector<kui::systemWM::SysWindow*> ActiveWindows;

// SDL kind of breaks when managing multiple windows on multiple threads :(
// This should be okay for a game engine though, since we usually only have a single window anyways.
static void MainThreadBlocking(std::function<void()> fn)
{
	if (engine::thread::IsMainThread)
	{
		fn();
	}
	else
	{
		auto HeapFunction = new std::function<void()>(fn);
		SDL_RunOnMainThread([](void* Data) {
			auto Ptr = reinterpret_cast<std::function<void()>*>(Data);
			(*Ptr)();
			delete Ptr;
		}, HeapFunction, true);
	}
}

kui::systemWM::SysWindow* kui::systemWM::NewWindow(
	Window* Parent, Vec2ui Size, Vec2ui Pos, std::string Title, Window::WindowFlag Flags)
{
	engine::platform::Init();

	SysWindow* OutWindow = new SysWindow();
	OutWindow->Parent = Parent;

	int SDLFlags = SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN;

	if ((Flags & Window::WindowFlag::Resizable) == Window::WindowFlag::Resizable)
	{
		SDLFlags |= SDL_WINDOW_RESIZABLE;
	}

	if ((Flags & Window::WindowFlag::AlwaysOnTop) == Window::WindowFlag::AlwaysOnTop)
	{
		SDLFlags |= SDL_WINDOW_ALWAYS_ON_TOP;
	}

	OutWindow->IsEngineWindow = (Window::WindowFlag(engine::EngineWindowFlag::EngineWindow) & Flags)
		!= Window::WindowFlag::None;

	if (OutWindow->IsEngineWindow)
	{
		SDLFlags |= SDL_WINDOW_MAXIMIZED;
	}
	if ((Flags & Window::WindowFlag::Popup) == Window::WindowFlag::Popup)
	{
		OutWindow->ShouldRaise = true;
	}

	MainThreadBlocking([=]() {

		std::lock_guard g{ WindowCreateMutex };

		OutWindow->SDLWindow = SDL_CreateWindow(Title.c_str(), int(Size.X), int(Size.Y), SDLFlags);
		if (ActiveWindows.size() && (Flags & Window::WindowFlag::Popup) == Window::WindowFlag::Popup)
		{
			SDL_SetWindowParent(OutWindow->SDLWindow, ActiveWindows[0]->SDLWindow);
			SDL_SetWindowModal(OutWindow->SDLWindow, true);
		}

		if (!OutWindow->IsEngineWindow)
		{
			Vec2f OldSize = GetWindowSize(OutWindow);
			Vec2f NewSize = OldSize * GetDPIScale(OutWindow);

			SetWindowSize(OutWindow, NewSize);

			SetWindowPosition(OutWindow, Pos - (NewSize - OldSize) / 2);
		}

		OutWindow->IsMain = ActiveWindows.empty();
		ActiveWindows.push_back(OutWindow);
	});

	engine::platform::InitWindow(OutWindow, int(Flags));
	OutWindow->GLContext = SDL_GL_CreateContext(OutWindow->SDLWindow);

	SDL_StartTextInput(OutWindow->SDLWindow);
	SDL_SetTextInputArea(OutWindow->SDLWindow, NULL, 0);

	OutWindow->WindowCursors = {
		SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_DEFAULT),
		SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_POINTER),
		SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_TEXT),
		SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NS_RESIZE),
		SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_EW_RESIZE),
	};

	SDL_GL_MakeCurrent(OutWindow->SDLWindow, OutWindow->GLContext);

#if WINDOWS

	if (OutWindow->IsMain && engine::Engine::Instance)
	{
		auto eventWatch = [](void* userdata, SDL_Event* event) -> bool {
			auto win = (SysWindow*)userdata;
			if (event->window.windowID != SDL_GetWindowID(win->SDLWindow))
			{
				return true;
			}
			if (event->type == SDL_EVENT_WINDOW_RESIZED)
			{
				win->Parent->OnResized();
				win->Parent->RedrawInternal();
				engine::VideoSubsystem::Current->RenderUpdate();
			}
			return true;
		};

		SDL_AddEventWatch(eventWatch, OutWindow);
	}

#endif

	return OutWindow;
}

using namespace kui;
static std::map<int, kui::Key> Keys =
{
	{ SDLK_ESCAPE, Key::ESCAPE },
	{ SDLK_BACKSPACE, Key::BACKSPACE },
	{ SDLK_TAB, Key::TAB },
	{ SDLK_SPACE, Key::SPACE },
	{ SDLK_DELETE, Key::DELETE },
	{ SDLK_PLUS, Key::PLUS },
	{ SDLK_COMMA, Key::COMMA },
	{ SDLK_PERIOD, Key::PERIOD },
	{ SDLK_SLASH, Key::SLASH },
	{ SDLK_0, Key::k0 },
	{ SDLK_1, Key::k1 },
	{ SDLK_2, Key::k2 },
	{ SDLK_3, Key::k3 },
	{ SDLK_4, Key::k4 },
	{ SDLK_5, Key::k5 },
	{ SDLK_6, Key::k6 },
	{ SDLK_7, Key::k7 },
	{ SDLK_8, Key::k8 },
	{ SDLK_9, Key::k9 },
	{ SDLK_F1, Key::F1 },
	{ SDLK_F2, Key::F2 },
	{ SDLK_F3, Key::F3 },
	{ SDLK_F4, Key::F4 },
	{ SDLK_F5, Key::F5 },
	{ SDLK_F6, Key::F6 },
	{ SDLK_F7, Key::F7 },
	{ SDLK_F8, Key::F8 },
	{ SDLK_F9, Key::F9 },
	{ SDLK_F10, Key::F10 },
	{ SDLK_F11, Key::F11 },
	{ SDLK_F12, Key::F12 },
	{ SDLK_SEMICOLON, Key::SEMICOLON },
	{ SDLK_LESS, Key::LESS },
	{ SDLK_RETURN, Key::RETURN },
	{ SDLK_LEFTBRACKET, Key::LEFTBRACKET },
	{ SDLK_RIGHTBRACKET, Key::RIGHTBRACKET },
	{ SDLK_RIGHT, Key::RIGHT },
	{ SDLK_LEFT, Key::LEFT },
	{ SDLK_UP, Key::UP },
	{ SDLK_DOWN, Key::DOWN },
	{ SDLK_LSHIFT, Key::LSHIFT },
	{ SDLK_RSHIFT, Key::LSHIFT },
	{ SDLK_LCTRL, Key::LCTRL },
	{ SDLK_RCTRL, Key::LCTRL },
	{ SDLK_LALT, Key::LALT },
	{ SDLK_RALT, Key::LALT },
	{ SDLK_A, Key::a },
	{ SDLK_B, Key::b },
	{ SDLK_C, Key::c },
	{ SDLK_D, Key::d },
	{ SDLK_E, Key::e },
	{ SDLK_F, Key::f },
	{ SDLK_G, Key::g },
	{ SDLK_H, Key::h },
	{ SDLK_I, Key::i },
	{ SDLK_J, Key::j },
	{ SDLK_K, Key::k },
	{ SDLK_L, Key::l },
	{ SDLK_M, Key::m },
	{ SDLK_N, Key::n },
	{ SDLK_O, Key::o },
	{ SDLK_P, Key::p },
	{ SDLK_Q, Key::q },
	{ SDLK_R, Key::r },
	{ SDLK_S, Key::s },
	{ SDLK_T, Key::t },
	{ SDLK_U, Key::u },
	{ SDLK_V, Key::v },
	{ SDLK_W, Key::w },
	{ SDLK_X, Key::x },
	{ SDLK_Y, Key::y },
	{ SDLK_Z, Key::z },
};

void kui::systemWM::DestroyWindow(SysWindow* Target)
{
	SDL_GL_DestroyContext(Target->GLContext);

	MainThreadBlocking([=]() {

		for (auto i = ActiveWindows.begin(); i < ActiveWindows.end(); i++)
		{
			if (*i == Target)
			{
				ActiveWindows.erase(i);
				break;
			}
		}

		for (auto& [Key, IsPressed] : Target->Parent->Input.PressedKeys)
		{
			if (!IsPressed)
			{
				break;
			}

			SDL_Event KeyReleasedEvent;
			for (auto& win : ActiveWindows)
			{
				KeyReleasedEvent.type = SDL_EVENT_KEY_UP;
				for (auto& [SDLKey, KuiKey] : Keys)
				{
					if (KuiKey == Key)
					{
						KeyReleasedEvent.key.key = SDLKey;
						break;
					}
				}
				win->Events.push_back(KeyReleasedEvent);
			}
		}

		for (SDL_Cursor* Cursor : Target->WindowCursors)
		{
			SDL_DestroyCursor(Cursor);
		}

		SDL_DestroyWindow(Target->SDLWindow);
		delete Target;
	});
}

void kui::systemWM::SwapWindow(SysWindow* Target)
{
	if (Target->ShouldShow)
	{
		Target->ShouldShow = false;
		SDL_ShowWindow(Target->SDLWindow);
	}
	if (Target->ShouldRaise)
	{
		Target->ShouldRaise = false;
		SDL_RaiseWindow(Target->SDLWindow);
	}
	if (!Target->IsMain || !engine::Engine::Instance)
	{
		SDL_GL_SetSwapInterval(1);
		SDL_GL_SwapWindow(Target->SDLWindow);
	}
	// Don't swap the main window, the engine does that elsewhere.
}

void kui::systemWM::WaitFrame(SysWindow* Target, float RemainingTime)
{
	if (!Target->IsMain || !engine::Engine::Instance)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(int(RemainingTime * 500)));
	}
}

void* kui::systemWM::GetPlatformHandle(SysWindow* Target)
{
	return Target->SDLWindow;
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
	if (engine::thread::IsMainThread)
	{
		SDL_Event ev;
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
	static std::mutex m;

	std::lock_guard g{ m };
	Target->UpdateEvents();
}

bool kui::systemWM::WindowHasFocus(SysWindow* Target)
{
	return SDL_GetKeyboardFocus() == Target->SDLWindow;
}

bool kui::systemWM::WindowHasMouseFocus(SysWindow* Target)
{
	return SDL_GetMouseFocus() == Target->SDLWindow;
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
	std::string Out = Target->TextInput;
	Target->TextInput.clear();
	return Out;
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
	SDL_SetCursor(Target->WindowCursors[size_t(NewCursor)]);
}

float kui::systemWM::GetDPIScale(SysWindow* Target)
{
	float Density = SDL_GetDisplayContentScale(SDL_GetDisplayForWindow(Target->SDLWindow));

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

	return State & SDL_BUTTON_LMASK;
}

bool kui::systemWM::IsRMBDown()
{
	SDL_MouseButtonFlags State = SDL_GetGlobalMouseState(nullptr, nullptr);
	return State & SDL_BUTTON_RMASK;
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
	auto Maximized = SDL_GetWindowFlags(Target->SDLWindow) & SDL_WINDOW_MAXIMIZED;
	return Maximized;
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
	if (!engine::platform::ShowMessageBox(Title, Text, Type))
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_BUTTONS_RIGHT_TO_LEFT, Title.c_str(), Text.c_str(), nullptr);
	}
}

bool kui::systemWM::YesNoBox(std::string, std::string)
{
	return false;
}

std::string kui::systemWM::SelectFileDialog(bool)
{
	return "";
}

void kui::systemWM::SysWindow::HandleKey(SDL_Keycode k, bool IsDown)
{
	using namespace engine::subsystem;
	using namespace engine;

	if (k == SDLK_TAB && IsDown)
	{
		TextInput += "\t";
	}

	Parent->Input.SetKeyDown(Keys[k], IsDown);

	if (!Engine::Instance)
	{
		return;
	}

	InputSubsystem* InputSys = Engine::GetSubsystem<InputSubsystem>();
	InputSys->SetKeyDown(input::Key(k), IsDown);
}

void kui::systemWM::SysWindow::UpdateEvents()
{
	using namespace engine;

	std::vector<SDL_Event> EventsCopy;
	EventsCopy = Events;
	Events.clear();

	input::MouseMovement = 0;

	for (auto& ev : EventsCopy)
	{
		switch (ev.type)
		{
		case SDL_EVENT_WINDOW_RESIZED:
			Parent->OnResized();
			break;
		case SDL_EVENT_TEXT_INPUT:
			if (!this->Parent->Input.IsKeyDown(Key::LCTRL))
			{
				TextInput += ev.text.text;
			}
			break;
		case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
			Parent->Close();
			break;
		case SDL_EVENT_MOUSE_BUTTON_DOWN:
			Parent->Input.UpdateCursorPosition();
			break;
		case SDL_EVENT_MOUSE_WHEEL:
			Parent->Input.MoveMouseWheel(int(ev.wheel.y));
			break;
		case SDL_EVENT_KEY_DOWN:
			HandleKey(ev.key.key, true);
			break;
		case SDL_EVENT_KEY_UP:
			HandleKey(ev.key.key, false);
			break;
		case SDL_EVENT_MOUSE_MOTION:
			if (WindowHasFocus(this) && !input::ShowMouseCursor)
			{
				input::MouseMovement = input::MouseMovement
					+ Vector2(ev.motion.xrel, ev.motion.yrel) * Vector2(0.05f, 0.05f);
			}
			break;
		default:
			break;
		}
	}
}
#else
#error Server unsupported right now
#endif