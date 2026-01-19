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

	bool HasShortcut = false;

	for (auto& i : Options)
	{
		if (!i.Shortcut.empty())
		{
			HasShortcut = true;
		}
	}

	Box = (new UIBlurBackground(false, Position, 1, 0))
		->SetCorner(EditorUI::Theme.CornerSize);

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

		bool HasImage = !i->Icon.empty();

		auto Button = new UIButton(true, 0, i->Name == "Delete" ? Vec3f(0.5f, 0, 0) : EditorUI::Theme.LightBackground, fn);

		if (HasImage)
		{
			Button->AddChild((new UIBackground(true, 0, EditorUI::Theme.Text, 16_px))
				->SetUseTexture(true, i->Icon)
				->SetPadding(0, 0, 6_px, 6_px));
		}

		Box->AddChild(Button
			->SetBorder(1_px, EditorUI::Theme.BackgroundHighlight)
			->SetBorderEdges(First, Last || i->Separator, true, true)
			->SetCorner(EditorUI::Theme.CornerSize)
			->SetOpacity(0.65f)
			->SetCorners(First, First, Last, Last)
			->SetMinSize(SizeVec::Pixels(220, 24))
			->SetVerticalAlign(UIBox::Align::Centered)
			->AddChild((new UIText(11_px, EditorUI::Theme.Text, i->Name, EditorUI::EditorFont))
				->SetTextWidthOverride(HasShortcut ? 140_px : 190_px)
				->SetWrapEnabled(true, HasShortcut ? 140_px : 190_px)
				->SetPadding(3_px, 3_px, HasImage ? 0 : 28_px, 3_px)));

		if (HasShortcut)
		{
			Button->AddChild((new UIBox(true))
				->SetPadding(4_px)
				->SetMinWidth(50_px)
				->SetHorizontalAlign(UIBox::Align::Reverse)
				->AddChild((new UIText(10_px, EditorUI::Theme.DarkText, i->Shortcut, EditorUI::EditorFont))
					->SetWrapEnabled(true, 50_px)));
		}
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
