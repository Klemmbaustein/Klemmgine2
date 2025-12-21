#pragma once
#include <Core/Types.h>
#include <functional>
#include <kui/UI/UIButton.h>

namespace engine::editor
{
	class DropdownMenu
	{
	public:
		struct Option
		{
			string Name;
			string Shortcut;
			string Icon;
			std::function<void()> OnClicked;
			bool Separator = false;
		};

		DropdownMenu(std::vector<Option> Options, kui::Vec2f Position);
		~DropdownMenu();

		static void UpdateDropdowns();
		static thread_local DropdownMenu* Current;

		kui::UIBox* Box = nullptr;
		static void Clear();
	};
}
