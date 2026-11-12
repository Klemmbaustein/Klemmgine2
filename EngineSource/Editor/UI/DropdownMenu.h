#pragma once
#include <Core/Types.h>
#include <functional>
#include <kui/UI/UIButton.h>
#include <Engine/UI/Popup.h>

namespace engine::editor
{
	class DropdownMenu : Popup
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

		DropdownMenu(std::vector<Option> Options, kui::Vec2f Position, bool AlignUp = false)
			: DropdownMenu(Options, Position, AlignUp, true, 0)
		{

		}
		~DropdownMenu();

	private:
		DropdownMenu(std::vector<Option> Options, kui::Vec2f Position, bool AlignUp, bool RemoveOld, size_t Depth);
	};
}
