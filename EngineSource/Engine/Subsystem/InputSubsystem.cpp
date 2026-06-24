#include "InputSubsystem.h"
#include <Engine/Graphics/VideoSubsystem.h>
#include <Engine/Engine.h>
#include <Engine/Input.h>
#include <SDL3/SDL.h>
#include <Engine/Internal/SystemWM_SDL3.h>

engine::subsystem::InputSubsystem::InputSubsystem()
	: Subsystem("Input", Log::LogColor::Magenta)
{
	THIS_SUBSYSTEM_DEPENDS_ON(VideoSubsystem);
	input::ShowMouseCursor = false;
}

void engine::subsystem::InputSubsystem::NextInputUpdate()
{
	InputFrame++;
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

bool engine::subsystem::InputSubsystem::IsKeyHeld(input::Key Key)
{
	if (PressedKeys.contains(Key))
		return PressedKeys[Key].IsPressed;
	return false;
}

bool engine::subsystem::InputSubsystem::IsKeyPressed(input::Key Key)
{
	if (PressedKeys.contains(Key))
		return PressedKeys[Key].ChangedFrame == InputFrame && PressedKeys[Key].IsPressed;
	return false;
}

bool engine::subsystem::InputSubsystem::IsKeyReleased(input::Key Key)
{
	if (PressedKeys.contains(Key))
		return PressedKeys[Key].ChangedFrame == InputFrame && !PressedKeys[Key].IsPressed;
	return false;
}

void engine::subsystem::InputSubsystem::SetKeyDown(input::Key Key, bool Value)
{
	PressedKeys[Key] = KeyData{
		.IsPressed = Value,
		.ChangedFrame = InputFrame,
	};
}
