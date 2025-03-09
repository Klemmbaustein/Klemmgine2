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

bool engine::input::IsKeyDown(Key k)
{
	if (kui::Window::GetActiveWindow()->Input.PollForText)
		return false;

	return Engine::GetSubsystem<subsystem::InputSubsystem>()->KeyDown(k);
}