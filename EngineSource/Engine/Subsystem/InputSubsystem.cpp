#include "InputSubsystem.h"
#include "VideoSubsystem.h"
#include <Engine/Engine.h>
#include <Engine/Input.h>
#include <SDL3/SDL.h>
#include <Engine/Internal/SystemWM_SDL3.h>

engine::subsystem::InputSubsystem::InputSubsystem()
	: Subsystem("Input", Log::LogColor::Magenta)
{
	THIS_SUBSYSTEM_DEPENDS_ON(VideoSubsystem);
#if !EDITOR
	input::ShowMouseCursor = false;
#else
	input::ShowMouseCursor = true;
#endif
}

void engine::subsystem::InputSubsystem::Update()
{
	VideoSubsystem* VideoSys = Engine::GetSubsystem<VideoSubsystem>();

	if (CursorVisible != input::ShowMouseCursor)
	{
		SDL_Window* SDLWindow = static_cast<kui::systemWM::SysWindow*>(VideoSys->MainWindow->GetSysWindow())->SDLWindow;
		if (input::ShowMouseCursor)
			SDL_SetWindowRelativeMouseMode(SDLWindow, false);
		else
			SDL_SetWindowRelativeMouseMode(SDLWindow, true);
		CursorVisible = input::ShowMouseCursor;
	}
}

bool engine::subsystem::InputSubsystem::KeyDown(input::Key Key)
{
	if (PressedKeys.contains(Key))
		return PressedKeys[Key];
	return false;
}

void engine::subsystem::InputSubsystem::SetKeyDown(input::Key Key, bool Value)
{
	if (PressedKeys.contains(Key))
		PressedKeys[Key] = Value;
	PressedKeys.insert({ Key, Value });
}

bool engine::subsystem::InputSubsystem::KeyPressed(input::Key Key)
{
	if (PressedKeys.contains(Key))
		return PressedKeys[Key];
	return false;
}
