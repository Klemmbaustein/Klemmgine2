#include "Input.h"
#include <Engine/Engine.h>
#include <Engine/Subsystem/InputSubsystem.h>

bool engine::input::ShowMouseCursor = false;
engine::Vector2 engine::input::MouseMovement = 0;

extern bool engine::input::IsLMBDown = false;
extern bool engine::input::IsLMBClicked = false;
extern bool engine::input::IsRMBDown = false;
extern bool engine::input::IsRMBClicked = false;

bool engine::input::IsKeyDown(Key k)
{
	return Engine::GetSubsystem<subsystem::InputSubsystem>()->KeyDown(k);
}