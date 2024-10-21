#pragma once
#include "Subsystem.h"
#include <Engine/Input.h>
#include <map>

namespace engine::subsystem
{
	class InputSubsystem : public Subsystem
	{
		std::map<input::Key, bool> PressedKeys;
		bool CursorVisible = true;
	public:
		InputSubsystem();

		void Update() override;

		bool KeyDown(input::Key Key);
		void SetKeyDown(input::Key Key, bool Value);
		bool KeyPressed(input::Key Key);
	};
}