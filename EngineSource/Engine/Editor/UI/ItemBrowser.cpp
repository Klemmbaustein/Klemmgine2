#ifdef EDITOR
#include "ItemBrowser.h"
#include <iostream>
#include <Engine/Input.h>
using namespace kui;

engine::editor::ItemBrowser::ItemBrowser(string Name, string InternalName)
	: EditorPanel(Name, InternalName)
{
	Heading = new ItemBrowserHeading();
	Background->AddChild(Heading);
	ItemsScrollBox = new UIScrollBox(false, 0, true);
	Background->AddChild(ItemsScrollBox);

	Heading->backButton->OnClicked = [this]() { Back(); };
	Heading->searchBox->OnClickedFunction = [this]()
		{
			Filter = Heading->searchBox->GetText();
			UpdateItems();
		};
}

void engine::editor::ItemBrowser::Update()
{
	ItemsScrollBox->SetMinSize(Background->GetUsedSize() - Vec2f(0, Heading->GetUsedSize().Y)
		- Vec2f(0, UIBox::PixelSizeToScreenSize(25, ItemsScrollBox->GetParentWindow()).Y));
	ItemsScrollBox->SetMaxSize(ItemsScrollBox->GetMinSize());
	Heading->SetPathWrapping(Background->GetUsedSize().X - UIBox::PixelSizeToScreenSize(60, Background->GetParentWindow()).X);

	if (input::IsRMBClicked)
	{
		for (auto& i : Buttons)
		{
			if (i.second->IsBeingHovered() && i.first.OnRightClick)
			{
				i.first.OnRightClick();
				return;
			}
		}
	}
}
void engine::editor::ItemBrowser::OnResized()
{
	Background->UpdateElement();
	UpdateItems();
}
void engine::editor::ItemBrowser::UpdateItems()
{
	Heading->SetPathText("Assets/" + Path);
	ItemsScrollBox->DeleteChildren();
	Buttons.clear();

	std::vector<Item> NewItems = GetItems();

	UIBox* CurrentBox = new UIBox(true, 0);
	ItemsScrollBox->AddChild(CurrentBox);

	size_t ItemsPerRow = size_t(
		Background->GetUsedSize().X
		/ UIBox::PixelSizeToScreenSize(90, Background->GetParentWindow()).X);

	if (ItemsPerRow == 0)
		return;

	size_t Column = 0;
	for (size_t i = 0; i < NewItems.size(); i++)
	{
		Item NewItem = NewItems[i];

		if (!Filter.empty() && NewItem.Name.find(Filter) == std::string::npos)
			continue;

		auto* btn = new ItemBrowserButton();
		btn->SetColor(NewItem.Color);
		btn->SetName(NewItem.Name);
		btn->SetImage(NewItem.Image);
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
				}
				else
				{
					Buttons[i].first.Selected = false;
					if (Buttons[i].first.OnClick)
						Buttons[i].first.OnClick();
				}
			};
		Buttons.push_back({ NewItem, btn });
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