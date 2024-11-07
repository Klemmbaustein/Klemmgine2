#pragma once
#include <Engine/Types.h>
#include <functional>
#include <kui/UI/UIButton.h>

namespace engine::editor
{
	class DropdownMenu
	{
	public:
		struct Option
		{
			std::function<void()> OnClicked;
			string Name;
		};

		DropdownMenu(std::vector<Option> Options, kui::Vec2f Position);
		~DropdownMenu();

		static void UpdateDropdowns();

		kui::UIBox* Box = nullptr;

	private:
		static DropdownMenu* Current;
	};
}