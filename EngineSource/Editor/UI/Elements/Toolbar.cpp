#include "Toolbar.h"
#include <Editor/UI/EditorUI.h>
#include <kui/Window.h>

using namespace kui;

engine::editor::Toolbar::Toolbar(bool Padded, kui::Vec3f Color)
	: UIBackground(true, 0, Color, SizeVec(UISize::Parent(1), 40_px))
{
	if (Padded)
		SetPadding(1_px, 0, 1_px, 1_px);
	else
	{
		SetPadding(0, 0, 0, 0);
	}
	SetBorder(1_px, EditorUI::Theme.BackgroundHighlight);
	SetBorderEdges(false, true, false, false);

	if (Color == Vec3f(-1))
	{
		SetColor(EditorUI::Theme.Background);
		GetParentWindow()->Markup.ListenToGlobal("Color_Text", AnyContainer(), this, [this]()
		{
			SetBorder(1_px, EditorUI::Theme.BackgroundHighlight);
			SetColor(EditorUI::Theme.Background);

			for (auto& i : this->Buttons)
			{
				i->SetColor(EditorUI::Theme.Background);
			}
		});
	}
}

engine::editor::Toolbar::~Toolbar()
{
	GetParentWindow()->Markup.RemoveGlobalListener(this);
}

void engine::editor::Toolbar::AddButton(string Name, string Icon, std::function<void()> OnClicked)
{
	auto Button = new ToolBarButton();
	Button->SetName(Name);
	Button->SetIcon(Icon);
	Button->SetColor(this->GetColor());
	Button->btn->OnClicked = OnClicked;
	Button->dropdownButton->IsCollapsed = true;
	AddChild(Button);
	this->Buttons.push_back(Button);
}

void engine::editor::Toolbar::AddDropdown(string Name, string Icon, std::vector<DropdownMenu::Option> Options)
{
	auto Button = new ToolBarButton();
	Button->SetName(Name);
	Button->SetIcon(Icon);
	Button->SetColor(this->GetColor());
	Button->btn->OnClicked = [Options, Button]()
		{
			new DropdownMenu(Options, Button->btn->GetPosition());
		};
	AddChild(Button);
	this->Buttons.push_back(Button);
}

void engine::editor::Toolbar::AddDropdown(string Name, string Icon,
	std::function<std::vector<DropdownMenu::Option>()> Options)
{
	auto Button = new ToolBarButton();
	Button->SetName(Name);
	Button->SetIcon(Icon);
	Button->SetColor(this->GetColor());
	Button->btn->OnClicked = [Options, Button]()
	{
		new DropdownMenu(Options(), Button->btn->GetPosition());
	};
	AddChild(Button);
	this->Buttons.push_back(Button);
}

void engine::editor::Toolbar::SetToolbarColor(kui::Vec3f NewColor)
{
	SetColor(NewColor);

	for (auto& i : this->Buttons)
	{
		i->SetColor(NewColor);
	}
}