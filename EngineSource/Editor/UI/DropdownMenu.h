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
			std::vector<Option> SubMenu;
			bool Separator = false;
		};

		DropdownMenu(std::vector<Option> Options, kui::Vec2f Position)
			: DropdownMenu(Options, Position, true)
		{

		}
		~DropdownMenu();

		static void UpdateDropdowns();
		static thread_local std::vector<DropdownMenu*> Current;

		kui::UIBox* Box = nullptr;
		static void Clear();

	private:
		DropdownMenu(std::vector<Option> Options, kui::Vec2f Position, bool RemoveOld);
	};
}
