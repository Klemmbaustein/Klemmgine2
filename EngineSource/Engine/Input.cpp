#include "Input.h"
#include <Engine/Engine.h>
#include <Engine/Subsystem/InputSubsystem.h>

bool engine::input::ShowMouseCursor = false;
engine::Vector2 engine::input::MouseMovement = 0;

bool engine::input::IsKeyDown(Key k)
{
	return Engine::GetSubsystem<subsystem::InputSubsystem>()->KeyDown(k);
}