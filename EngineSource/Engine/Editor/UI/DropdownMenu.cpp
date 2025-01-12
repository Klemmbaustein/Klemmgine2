#ifdef EDITOR
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

thread_local engine::editor::DropdownMenu* engine::editor::DropdownMenu::Current = nullptr;

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
				->SetBorder(UISize::Pixels(1), EditorUI::Theme.DarkBackground)
				->SetBorderEdges(i == Options.begin(), i == Options.end() - 1 || i->Separator, true, true)
				->SetMinSize(SizeVec::Pixels(150, 24))
				->SetVerticalAlign(UIBox::Align::Centered)
				->AddChild((new UIText(UISize::Pixels(12), EditorUI::Theme.Text, i->Name, EditorUI::EditorFont))
					->SetPadding(UISize::Pixels(5))));
	}

	Window::GetActiveWindow()->Input.KeyboardFocusTargetBox = Box;

	Box->UpdateElement();

	Box->SetPosition((Box->GetPosition() - Vec2f(0, Box->GetUsedSize().GetScreen().Y))
		.Clamp(-1, Vec2f(1) - Box->GetUsedSize().GetScreen()));

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
#endif