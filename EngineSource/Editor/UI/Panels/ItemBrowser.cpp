#ifdef EDITOR
#include "ItemBrowser.h"
#include <Editor/UI/EditorUI.h>
#include <kui/Window.h>
#include <Engine/Input.h>
#include <Editor/UI/Elements/DroppableBox.h>
#include <Core/Log.h>
#include <filesystem>
using namespace kui;

engine::editor::ItemBrowser::ItemBrowser(string Name, string InternalName)
	: EditorPanel(Name, InternalName)
{
	Heading = new ItemBrowserHeading();
	Background->AddChild(Heading);
	ItemsScrollBox = new UIScrollBox(false, 0, true);

	StatusText = new UIText(11_px, EditorUI::Theme.Text, "", EditorUI::EditorFont);
	StatusText
		->SetPadding(5_px);

	Background->AddChild((new DroppableBox(false, [this](EditorUI::DraggedItem Item)
		{
			auto* btn = GetHoveredButton();

			if (!btn)
				return;

			if (!std::filesystem::exists(btn->first.Path))
				return;

			if (Item.Name == btn->first.Name)
				return;

			if (btn->first.IsDirectory && std::filesystem::exists(btn->first.Path))
			{
				try
				{
					std::string FileName = Item.Path.substr(Item.Path.find_last_of("/\\") + 1);
					std::filesystem::rename(Item.Path, btn->first.Path + "/" + FileName);
				}
				catch (std::filesystem::filesystem_error e)
				{
					Log::Error(e.what());
				}
			}
			UpdateItems();
		}))
		->SetMinSize(SizeVec(UISize(1, SizeMode::ParentRelative)))
		->SetMaxSize(SizeVec(UISize(1, SizeMode::ParentRelative)))
		->AddChild(ItemsScrollBox)
		->AddChild(StatusText));


	Heading->backButton->OnClicked = [this]() { Back(); };
	Heading->searchBox->field->OnChanged = [this]()
		{
			Filter = Heading->searchBox->field->GetText();
			UpdateItems();
		};
}

void engine::editor::ItemBrowser::Update()
{
	if (!Visible)
		return;

	auto Win = Background->GetParentWindow();

	ItemsScrollBox->SetMinSize(Background->GetUsedSize().GetScreen() - Vec2f(0, Heading->GetUsedSize().GetScreen().Y)
		- SizeVec(1_px, 25_px).GetScreen());
	ItemsScrollBox->SetMaxSize(ItemsScrollBox->GetMinSize());
	Heading->SetPathWrapping(UISize::Pixels(Background->GetUsedSize().GetPixels().X - 60));

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
			this->OnBackgroundRightClick(Win->Input.MousePosition);
		}
	}
}

void engine::editor::ItemBrowser::OnResized()
{
	Background->UpdateElement();
	Heading->searchBox->SetSize(UISize::Screen(Size.X -
		UIBox::PixelSizeToScreenSize(33, Heading->GetParentWindow()).X));
	UpdateItems();
}

void engine::editor::ItemBrowser::UpdateItems()
{
	Heading->SetPathText(GetPathDisplayName());

	Buttons.clear();
	CurrentItems = GetItems();
	DisplayList();

}

void engine::editor::ItemBrowser::SetStatusText(string NewText)
{
	StatusText->SetText(NewText);
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
		Background->GetUsedSize().GetScreen().X
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
				EditorUI::Instance->StartAssetDrag(EditorUI::DraggedItem{
					.Name = NewItem.Name,
					.Type = "asset",
					.Path = NewItem.Path,
					.Icon = NewItem.Image,
					.Color = NewItem.Color,
					.ObjectType = NewItem.Type,
					});
			};

		btn->button->OnClicked = [this, i, btn]()
			{
				auto UpdateButtonSelected = [this, i](ItemBrowserButton* b)
					{
						b->SetBackgroundColor(Buttons[i].first.Selected ? EditorUI::Theme.HighlightDark : EditorUI::Theme.DarkBackground);
					};

				if (!Buttons[i].first.Selected)
				{
					if (!input::IsKeyDown(input::Key::LSHIFT))
					{
						for (auto& btn : Buttons)
						{
							if (btn.first.Selected)
							{
								btn.first.Selected = false;
								UpdateButtonSelected(btn.second);
							}
						}
					}
					Buttons[i].first.Selected = true;
					UpdateButtonSelected(btn);
				}
				else
				{
					Buttons[i].first.Selected = false;
					if (Buttons[i].first.OnClick)
						Buttons[i].first.OnClick();
					UpdateButtonSelected(btn);
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