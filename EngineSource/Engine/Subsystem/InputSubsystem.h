#pragma once
#include "Subsystem.h"
#include <Engine/Input.h>
#include <map>

namespace engine::subsystem
{
	class InputSubsystem : public Subsystem
	{
		struct KeyData
		{
			bool IsPressed = false;
			uint64 ChangedFrame = SIZE_MAX;
		};

		std::map<input::Key, KeyData> PressedKeys;
		bool CursorVisible = true;
	public:
		InputSubsystem();

		uint64 InputFrame = 0;

		void NextInputUpdate();

		void Update() override;

		bool IsKeyHeld(input::Key Key);
		bool IsKeyPressed(input::Key Key);
		bool IsKeyReleased(input::Key Key);
		void SetKeyDown(input::Key Key, bool Value);
	};
}