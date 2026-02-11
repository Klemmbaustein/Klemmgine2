#include "DropdownMenu.h"
#include "EditorUI.h"
#include <kui/UI/UIBlurBackground.h>
#include <kui/UI/UIText.h>
#include <Engine/Input.h>
#include <kui/Window.h>
using namespace kui;
using namespace engine::editor;

void engine::editor::DropdownMenu::Clear()
{
	for (auto& i : Current)
	{
		delete i;
	}
	Current.clear();
}

thread_local std::vector<DropdownMenu*> DropdownMenu::Current;

engine::editor::DropdownMenu::DropdownMenu(std::vector<Option> Options, kui::Vec2f Position, bool RemoveOld)
{
	if (RemoveOld)
	{
		Clear();
	}

	bool HasShortcut = false;

	for (auto& i : Options)
	{
		if (!i.Shortcut.empty() || !i.SubMenu.empty())
		{
			HasShortcut = true;
		}
	}

	Box = (new UIBlurBackground(false, Position, 1, 0))
		->SetCorner(EditorUI::Theme.CornerSize);

	for (auto i = Options.begin(); i < Options.end(); i++)
	{

		bool First = i == Options.begin();
		bool Last = i == Options.end() - 1;

		bool HasImage = !i->Icon.empty();

		Vec3f Color = i->Name == "Delete" ? Vec3f(0.5f, 0, 0) : EditorUI::Theme.LightBackground;

		auto Button = new UIButton(true, 0, Color, nullptr);

		Button->OnClicked = [Button, Option = *i]() {
			if (Option.OnClicked)
			{
				Clear();
				Option.OnClicked();
			}
			else if (!Option.SubMenu.empty())
			{
				Vec2f Position = Button->GetPosition() + Button->GetUsedSize().GetScreen();
				new DropdownMenu(Option.SubMenu, Position, false);
				return;
			}
			Clear();
		};

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
			auto Box = new UIBox(true);
			Button->AddChild(Box
				->SetPadding(4_px)
				->SetMinWidth(50_px)
				->SetHorizontalAlign(UIBox::Align::Reverse));

			if (!i->Shortcut.empty())
			{
				Box->AddChild((new UIText(10_px, EditorUI::Theme.DarkText, i->Shortcut, EditorUI::EditorFont))
					->SetWrapEnabled(true, 50_px));
			}
			else if (!i->SubMenu.empty())
			{
				Box->AddChild((new UIBackground(true, 0, EditorUI::Theme.Text, 16_px))
					->SetUseTexture(true, EditorUI::Asset("RightArrow.png")));
			}
		}
	}

	Window::GetActiveWindow()->Input.KeyboardFocusTargetBox = Box;

	Box->UpdateElement();

	float Height = Box->GetUsedSize().GetScreen().Y;

	Box->SetPosition((Box->GetPosition() - Vec2f(0, Height))
		.Clamp(-1, Vec2f(1) - Box->GetUsedSize().GetScreen()));

	Current.push_back(this);
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
	bool IsHovered = false;

	for (auto& i : Current)
	{
		if (i->Box->IsBeingHovered())
		{
			IsHovered = true;
		}
	}

	if (!Current.empty()
		&& (((input::IsLMBClicked || input::IsRMBClicked) && !IsHovered)
			|| input::IsKeyDown(input::Key::ESCAPE)))
	{
		Clear();
	}
}
