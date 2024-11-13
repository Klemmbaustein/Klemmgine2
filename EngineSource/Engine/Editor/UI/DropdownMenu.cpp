#include "DropdownMenu.h"
#include "EditorUI.h"
#include <kui/UI/UIText.h>
#include <Engine/Input.h>
#include <kui/Window.h>
#include <iostream>
using namespace kui;

void engine::editor::DropdownMenu::Clear()
{
	delete Current;
	Current = nullptr;
}

engine::editor::DropdownMenu* engine::editor::DropdownMenu::Current = nullptr;

engine::editor::DropdownMenu::DropdownMenu(std::vector<Option> Options, kui::Vec2f Position)
{
	if (Current)
	{
		delete Current;
		Current = nullptr;
	}

	Box = new UIBox(false, Position);

	for (auto i = Options.begin(); i < Options.end(); i++)
	{
		auto& OnClicked = i->OnClicked;
		Box
			->AddChild((new UIButton(true, 0, EditorUI::Theme.LightBackground, [OnClicked]() 
				{
					if (OnClicked)
						OnClicked();
					Clear();
				}))
				->SetBorder(1, UIBox::SizeMode::PixelRelative)
				->SetBorderColor(EditorUI::Theme.DarkBackground)
				->SetBorderEdges(i == Options.begin(), i == Options.end() - 1 || i->Separator, true, true)
				->SetMinSize(Vec2f(150, 24))
				->SetVerticalAlign(UIBox::Align::Centered)
				->SetSizeMode(UIBox::SizeMode::PixelRelative)
				->AddChild((new UIText(12, EditorUI::Theme.Text, i->Name, EditorUI::EditorFont))
					->SetTextSizeMode(UIBox::SizeMode::PixelRelative)
					->SetPadding(5)
					->SetPaddingSizeMode(UIBox::SizeMode::PixelRelative)));
	}

	Window::GetActiveWindow()->Input.KeyboardFocusTargetBox = Box;

	Box->UpdateElement();

	Box->SetPosition((Box->GetPosition() - Vec2f(0, Box->GetUsedSize().Y))
		.Clamp(-1, Vec2f(1) - Box->GetUsedSize()));

	Current = this;
}

engine::editor::DropdownMenu::~DropdownMenu()
{
	if (Window::GetActiveWindow()->Input.KeyboardFocusTargetBox == Box)
	{
		Window::GetActiveWindow()->Input.KeyboardFocusTargetBox = nullptr;
	}
	delete Box;
}

void engine::editor::DropdownMenu::UpdateDropdowns()
{
	if (Current
		&& (((input::IsLMBClicked || input::IsRMBClicked) && !Current->Box->IsBeingHovered())
			|| input::IsKeyDown(input::Key::ESCAPE)))
	{
		Clear();
	}
}
