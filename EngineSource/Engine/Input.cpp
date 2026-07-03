#include "Input.h"
#include <Engine/Engine.h>
#include <Engine/Subsystem/InputSubsystem.h>
#include <kui/Window.h>

bool engine::input::ShowMouseCursor = false;
engine::Vector2 engine::input::MouseMovement = 0;

bool engine::input::IsLMBDown = false;
bool engine::input::IsLMBClicked = false;
bool engine::input::IsRMBDown = false;
bool engine::input::IsRMBClicked = false;

bool engine::input::IsKeyHeld(Key k)
{
	if (!kui::Window::GetActiveWindow()
		|| kui::Window::GetActiveWindow()->Input.PollForText)
		return false;

	auto sys = Engine::GetSubsystem<subsystem::InputSubsystem>();

	return sys && sys->IsKeyHeld(k);
}

bool engine::input::IsKeyPressed(Key k)
{
	if (!kui::Window::GetActiveWindow()
		|| kui::Window::GetActiveWindow()->Input.PollForText)
		return false;

	auto sys = Engine::GetSubsystem<subsystem::InputSubsystem>();

	return sys && sys->IsKeyPressed(k);
}

bool engine::input::IsKeyReleased(Key k)
{
	if (!kui::Window::GetActiveWindow()
		|| kui::Window::GetActiveWindow()->Input.PollForText)
		return false;

	auto sys = Engine::GetSubsystem<subsystem::InputSubsystem>();

	return sys && sys->IsKeyReleased(k);
}
