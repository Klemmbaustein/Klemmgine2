#include "DropdownMenu.h"
#include "EditorUI.h"
#include <kui/UI/UIBlurBackground.h>
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

	Box = (new UIBlurBackground(false, Position, 1, 0))
		->SetCorner(5_px);

	for (auto i = Options.begin(); i < Options.end(); i++)
	{
		auto& OnClicked = i->OnClicked;
		auto fn = [OnClicked]()
			{
				if (OnClicked)
					OnClicked();
				Clear();
			};

		bool First = i == Options.begin();
		bool Last = i == Options.end() - 1;

		Box->AddChild((new UIButton(true, 0, i->Name == "Delete" ? Vec3f(0.5f, 0, 0) : EditorUI::Theme.LightBackground, fn))
			->SetBorder(1_px, EditorUI::Theme.BackgroundHighlight)
			->SetBorderEdges(First, Last || i->Separator, true, true)
			->SetCorner(5_px)
			->SetOpacity(0.65f)
			->SetCorners(First, First, Last, Last)
			->SetMinSize(SizeVec::Pixels(150, 24))
			->SetVerticalAlign(UIBox::Align::Centered)
			->AddChild((new UIText(11_px, EditorUI::Theme.Text, i->Name, EditorUI::EditorFont))
			->SetWrapEnabled(true, 140_px)
			->SetPadding(3_px, 3_px, 6_px, 3_px)));
	}

	Window::GetActiveWindow()->Input.KeyboardFocusTargetBox = Box;

	Box->UpdateElement();

	float Height = Box->GetUsedSize().GetScreen().Y;

	Box->SetPosition((Box->GetPosition() - Vec2f(0, Height))
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
