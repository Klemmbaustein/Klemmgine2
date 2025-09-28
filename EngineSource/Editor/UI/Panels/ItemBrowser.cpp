#ifdef EDITOR
#include "ItemBrowser.h"
#include <Editor/UI/EditorUI.h>
#include <kui/Window.h>
#include <Engine/Input.h>
#include <Editor/UI/Elements/DroppableBox.h>
#include <Core/Log.h>
#include <filesystem>
using namespace kui;
using namespace engine::editor;

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

	Heading->optionsButton->OnClicked = [this]()
	{
		bool IsList = this->Mode == DisplayMode::List;

		new DropdownMenu({
			DropdownMenu::Option{
				.Name = "Show icons",
				.Icon = !IsList ? EditorUI::Asset("Dot.png") : "",
				.OnClicked = [this]() {
					this->Mode = DisplayMode::Icons;
					this->LastItemsPerRow = 0;
					this->DisplayList();
				}
			},
			DropdownMenu::Option{
				.Name = "Show list",
				.Icon = IsList ? EditorUI::Asset("Dot.png") : "",
				.OnClicked = [this]() {
					this->Mode = DisplayMode::List;
					this->LastItemsPerRow = 0;
					this->DisplayList();
				}
			}, }, Heading->optionsButton->GetScreenPosition());
	};

	Heading->searchBox->field->OnChanged = [this]
	{
		Filter = str::Lower(Heading->searchBox->field->GetText());
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
			this->LastItemsPerRow = 0;
			DisplayList();
		}
	}

	if (input::IsRMBClicked && ScrollBoxHovered)
	{
		auto Selected = GetSelected();

		if (Selected.size() > 1)
		{
			this->OnItemsRightClick(Win->Input.MousePosition);
			return;
		}

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

	float Width = Background->GetUsedSize().GetPixels().X;

	if (Width > 600)
	{
		Heading->container->SetHorizontal(true);
		Heading->pathButtons->SetMinWidth(UISize::Pixels(Width - 300));
		Heading->searchBox->SetSize(UISize::Pixels(265));
		Heading->SetPathWrapping(UISize::Pixels(Background->GetUsedSize().GetPixels().X - 360));
	}
	else
	{
		Heading->container->SetHorizontal(false);
		Heading->pathButtons->SetMinWidth(UISize::Parent(1));
		Heading->searchBox->SetSize(UISize::Screen(Size.X -
			UIBox::PixelSizeToScreenSize(33, Heading->GetParentWindow()).X));
		Heading->SetPathWrapping(UISize::Pixels(Background->GetUsedSize().GetPixels().X - 60));
	}

	Heading->SetPathText(GetPathDisplayName());
	CurrentItems = GetItems();
	DisplayList();
}

void engine::editor::ItemBrowser::UpdateItems()
{
	Heading->SetPathText(GetPathDisplayName());

	CurrentItems = GetItems();
	LastItemsPerRow = 0;
	DisplayList();
}

std::vector<ItemBrowser::Item*> engine::editor::ItemBrowser::GetSelected()
{
	std::vector<Item*> Selected;

	for (auto& [Item, _] : Buttons)
	{
		if (Item.Selected)
		{
			Selected.push_back(&Item);
		}
	}

	return Selected;
}

void engine::editor::ItemBrowser::OnItemsRightClick(kui::Vec2f MousePosition)
{
}

void engine::editor::ItemBrowser::SetStatusText(string NewText)
{
	StatusText->SetText(NewText);
}

std::pair<engine::editor::ItemBrowser::Item, kui::UIBox*>* engine::editor::ItemBrowser::GetHoveredButton()
{
	for (auto& i : Buttons)
	{
		if (i.second && i.second->IsBeingHovered() && i.first.OnRightClick)
		{
			return &i;
		}
	}

	return nullptr;
}
void engine::editor::ItemBrowser::DisplayList()
{
	bool IsList = this->Mode == DisplayMode::List;

	size_t ItemsPerRow = size_t(
		(Background->GetUsedSize().GetPixels().X - 10.0f)
		/ (IsList ? 220 : 90));

	if (!IsList && ItemsPerRow == this->LastItemsPerRow)
	{
		return;
	}

	this->LastItemsPerRow = ItemsPerRow;
	ItemsScrollBox->DeleteChildren();
	Buttons.clear();

	if (ItemsPerRow == 0 && !IsList)
	{
		return;
	}
	else if (ItemsPerRow == 0)
	{
		ItemsPerRow++;
	}

	UIBox* CurrentBox = new UIBox(true, 0);
	ItemsScrollBox->AddChild(CurrentBox);

	UISize ElementSize = UISize::Pixels(std::round((Background->GetUsedSize().GetPixels().X - 10.0f) / ItemsPerRow) - 5.0f);

	size_t Column = 0;
	Buttons.resize(CurrentItems.size());
	for (size_t i = 0; i < CurrentItems.size(); i++)
	{
		Item NewItem = CurrentItems[i];
		NewItem.Selected = Buttons[i].first.Selected;

		if (!Filter.empty() && str::Lower(NewItem.Name).find(Filter) == std::string::npos)
			continue;

		UIButton* ElementButton = nullptr;
		UIBox* Element = nullptr;

		if (IsList)
		{
			auto* btn = new ItemBrowserListEntry();
			btn->SetBackgroundColor(NewItem.Selected ? EditorUI::Theme.HighlightDark : EditorUI::Theme.Background);
			btn->SetColor(NewItem.Color);
			btn->SetName(NewItem.Name);
			btn->SetMinWidth(ElementSize);
			btn->SetMaxWidth(ElementSize);
			btn->text->SetWrapEnabled(true, ElementSize.GetScreen().X - (20_px).GetScreen().X);
			btn->SetImage(NewItem.Image);
			btn->btn->OnDragged = [NewItem](int)
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
			Element = btn;
			ElementButton = btn->btn;
		}
		else
		{
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
			Element = btn;
			ElementButton = btn->button;
		}

		ElementButton->OnClicked = [this, i, Element, IsList]()
		{
			auto UpdateButtonSelected = [this, i, IsList](UIBox* b)
			{
				if (!IsList)
				{
					auto button = static_cast<ItemBrowserButton*>(b);
					button->SetBackgroundColor(
						Buttons[i].first.Selected ? EditorUI::Theme.HighlightDark : EditorUI::Theme.DarkBackground);
				}
				else
				{
					auto entry = static_cast<ItemBrowserListEntry*>(b);
					entry->btn->SetBorder(Buttons[i].first.Selected ? 1_px : 0, EditorUI::Theme.Highlight1);
					entry->SetBackgroundColor(
						Buttons[i].first.Selected ? EditorUI::Theme.HighlightDark : EditorUI::Theme.Background);
				}
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
				UpdateButtonSelected(Element);
			}
			else
			{
				Buttons[i].first.Selected = false;
				UpdateButtonSelected(Element);
				if (Buttons[i].first.OnClick)
					Buttons[i].first.OnClick();
			}
		};

		Buttons[i] = { NewItem, Element };

		CurrentBox->AddChild(Element);
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