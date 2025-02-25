#ifdef EDITOR
#pragma once
#include <Toolbar.kui.hpp>
#include <Core/Types.h>
#include <functional>
#include <Editor/UI/DropdownMenu.h>

namespace engine::editor
{
	class Toolbar : public kui::UIBackground
	{
	public:
		Toolbar(bool Padded = true);
		Toolbar(const Toolbar&) = delete;
		~Toolbar();

		void AddButton(string Name, string Icon, std::function<void()> OnClicked);
		void AddDropdown(string Name, string Icon, std::vector<DropdownMenu::Option> Options);
	};
}
#endif