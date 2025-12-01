#pragma once
#include <Toolbar.kui.hpp>
#include <Core/Types.h>
#include <functional>
#include <Editor/UI/DropdownMenu.h>
#include <Editor/UI/EditorUI.h>

namespace engine::editor
{
	class Toolbar : public kui::UIBackground
	{
	public:
		Toolbar(bool Padded = true, kui::Vec3f Color = -1);
		Toolbar(const Toolbar&) = delete;
		~Toolbar();

		void AddButton(string Name, string Icon, std::function<void()> OnClicked);
		void AddDropdown(string Name, string Icon, std::vector<DropdownMenu::Option> Options);

		void SetToolbarColor(kui::Vec3f NewColor);

	private:
		std::vector<ToolBarButton*> Buttons;
	};
}