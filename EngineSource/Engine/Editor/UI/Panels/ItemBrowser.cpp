#ifdef EDITOR
#include "ItemBrowser.h"
#include <Engine/Editor/UI/EditorUI.h>
#include <kui/Window.h>
#include <Engine/Input.h>
#include <Engine/Log.h>
using namespace kui;

engine::editor::ItemBrowser::ItemBrowser(string Name, string InternalName)
	: EditorPanel(Name, InternalName)
{
	Heading = new ItemBrowserHeading();
	Background->AddChild(Heading);
	ItemsScrollBox = new UIScrollBox(false, 0, true);
	Background->AddChild(ItemsScrollBox);

	Heading->backButton->OnClicked = [this]() { Back(); };
	Heading->searchBox->field->OnClickedFunction = [this]()
	{
		Filter = Heading->searchBox->field->GetText();
		UpdateItems();
	};
}

void engine::editor::ItemBrowser::Update()
{
	if (!Visible)
		return;

	ItemsScrollBox->SetMinSize(Background->GetUsedSize() - Vec2f(0, Heading->GetUsedSize().Y)
		- UIBox::PixelSizeToScreenSize(Vec2f(1, 25), ItemsScrollBox->GetParentWindow()));
	ItemsScrollBox->SetMaxSize(ItemsScrollBox->GetMinSize());
	Heading->SetPathWrapping(Background->GetUsedSize().X - UIBox::PixelSizeToScreenSize(60, Background->GetParentWindow()).X);

	bool ScrollBoxHovered = ItemsScrollBox->IsBeingHovered();

	if (input::IsLMBClicked && ScrollBoxHovered)
	{
		auto* Hovered = GetHoveredButton();
		if (!Hovered)
		{
			for (auto& i : Buttons)
			{
				i.first.Selected = false;
			}
			DisplayList();
		}
	}

	if (input::IsRMBClicked && ScrollBoxHovered)
	{
		auto* Hovered = GetHoveredButton();

		if (Hovered)
		{
			Hovered->first.OnRightClick();
		}
		else
		{
			this->OnBackgroundRightClick(ItemsScrollBox->GetParentWindow()->Input.MousePosition);
		}
	}
}
void engine::editor::ItemBrowser::OnResized()
{
	Background->UpdateElement();
	Heading->searchBox->field->SetSizeMode(UIBox::SizeMode::ScreenRelative);
	Heading->searchBox->SetSize(Vec2f(Size.X, 0) + UIBox::PixelSizeToScreenSize(Vec2f(-33, 22), Heading->GetParentWindow()));
	UpdateItems();
}
void engine::editor::ItemBrowser::UpdateItems()
{
	Heading->SetPathText(GetPathDisplayName());

	Buttons.clear();
	CurrentItems = GetItems();
	DisplayList();

}
std::pair<engine::editor::ItemBrowser::Item, ItemBrowserButton*>* engine::editor::ItemBrowser::GetHoveredButton()
{
	for (auto& i : Buttons)
	{
		if (i.second && i.second->button->IsBeingHovered() && i.first.OnRightClick)
		{
			return &i;
		}
	}

	return nullptr;
}
void engine::editor::ItemBrowser::DisplayList()
{
	ItemsScrollBox->DeleteChildren();
	UIBox* CurrentBox = new UIBox(true, 0);
	ItemsScrollBox->AddChild(CurrentBox);

	size_t ItemsPerRow = size_t(
		Background->GetUsedSize().X
		/ UIBox::PixelSizeToScreenSize(90, Background->GetParentWindow()).X);

	if (ItemsPerRow == 0)
		return;

	size_t Column = 0;
	Buttons.resize(CurrentItems.size());
	for (size_t i = 0; i < CurrentItems.size(); i++)
	{
		Item NewItem = CurrentItems[i];
		NewItem.Selected = Buttons[i].first.Selected;

		if (!Filter.empty() && NewItem.Name.find(Filter) == std::string::npos)
			continue;

		auto* btn = new ItemBrowserButton();
		btn->SetBackgroundColor(NewItem.Selected ? EditorUI::Theme.HighlightDark : EditorUI::Theme.DarkBackground);
		btn->SetColor(NewItem.Color);
		btn->SetName(NewItem.Name);
		btn->SetImage(NewItem.Image);
		btn->button->OnDragged = [NewItem](int)
		{
			EditorUI::Instance->StartDrag(EditorUI::DraggedItem{
				.Name = NewItem.Name,
				.Type = "asset",
				.Path = NewItem.Path,
				.Icon = NewItem.Image,
				.Color = NewItem.Color,
				});
		};

		btn->button->OnClicked = [this, i]()
		{
			if (!Buttons[i].first.Selected)
			{
				if (!input::IsKeyDown(input::Key::LSHIFT))
				{
					for (auto& btn : Buttons)
					{
						btn.first.Selected = false;
					}
				}
				Buttons[i].first.Selected = true;
				DisplayList();
			}
			else
			{
				Buttons[i].first.Selected = false;
				if (Buttons[i].first.OnClick)
					Buttons[i].first.OnClick();
				DisplayList();
			}
		};

		Buttons[i] = { NewItem, btn };
		CurrentBox->AddChild(btn);

		Column++;
		if (Column >= ItemsPerRow)
		{
			CurrentBox = new UIBox(true, 0);
			ItemsScrollBox->AddChild(CurrentBox);

			Column = 0;
		}
	}
}
#endif