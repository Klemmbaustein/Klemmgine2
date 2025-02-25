#ifdef EDITOR
#include "Toolbar.h"
#include <Editor/UI/EditorUI.h>

using namespace kui;

engine::editor::Toolbar::Toolbar(bool Padded)
	: UIBackground(true, 0, EditorUI::Theme.Background, SizeVec(UISize::Parent(1), 40_px))
{
	if (Padded)
		SetPadding(1_px, 0, 1_px, 1_px);
	SetBorder(1_px, EditorUI::Theme.BackgroundHighlight);
	SetBorderEdges(false, true, false, false);
}

engine::editor::Toolbar::~Toolbar()
{
}

void engine::editor::Toolbar::AddButton(string Name, string Icon, std::function<void()> OnClicked)
{
	auto Button = new ToolBarButton();
	Button->SetName(Name);
	Button->SetIcon(Icon);
	Button->btn->OnClicked = OnClicked;
	delete Button->dropdownButton;
	AddChild(Button);
}

void engine::editor::Toolbar::AddDropdown(string Name, string Icon, std::vector<DropdownMenu::Option> Options)
{
	auto Button = new ToolBarButton();
	Button->SetName(Name);
	Button->SetIcon(Icon);
	Button->btn->OnClicked = [Options, Button]()
		{
			new DropdownMenu(Options, Button->btn->GetPosition());
		};
	AddChild(Button);
}
#endif