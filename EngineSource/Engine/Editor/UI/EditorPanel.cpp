#ifdef EDITOR
#include "EditorPanel.h"
#include <kui/Window.h>
#include "EditorUI.h"
#include <iostream>
using namespace kui;

const float PanelPadding = 5;
const float TabsSize = 26;

engine::editor::EditorPanel::EditorPanel(string Name, string InternalName)
{
	if (InternalName != "panel")
	{
		PanelElement = new EditorPanelElement();
		Background = PanelElement->panelBackground;
	}
	ShouldUpdate = true;
	this->Name = Name;
}

engine::editor::EditorPanel::~EditorPanel()
{
}

void engine::editor::EditorPanel::UpdateLayout()
{
	if (!Parent)
	{
		UsedSize = EditorUI::Instance->MainBackground->GetMinSize();
		Position = EditorUI::Instance->MainBackground->GetPosition();
	}
	GenerateTabs();
	if (PanelElement)
	{
		Size = UsedSizeToPanelSize(UsedSize);
		PanelElement->SetSize(Size);
		PanelPosition = PositionToPanelPosition(Position);
		PanelElement->SetPosition(PanelPosition);
	}
	OnResized();

	if (ChildrenAlign != Align::Tabs)
	{
		float Pos = 0;
		float CurrentFraction = 0;
		float MinFraction = 0.1f;
		size_t it = 0;
		for (EditorPanel* Child : Children)
		{
			float PossibleFraction = Child->WidthFraction;
			float RequiredSize = (Children.size() - (it + 1)) * MinFraction;

			if (1 - CurrentFraction - PossibleFraction < RequiredSize)
			{
				PossibleFraction = 1 - RequiredSize - CurrentFraction;
			}

			if (Child == Children[Children.size() - 1])
			{
				PossibleFraction = 1 - CurrentFraction;
			}

			CurrentFraction += PossibleFraction;

			if (ChildrenAlign == Align::Horizontal)
			{
				Child->UsedSize = UsedSize * Vec2f(PossibleFraction, 1);
				Child->Position = Position + Vec2f(Pos, 0);
				Pos += Child->UsedSize.X;
			}
			else
			{
				Child->UsedSize = UsedSize * Vec2f(1, PossibleFraction);
				Child->Position = Position + Vec2f(0, Pos);
				Pos += Child->UsedSize.Y;
			}
			Child->UpdateLayout();
			it++;
		}
	}
	else
	{
		for (EditorPanel* Child : Children)
		{
			bool IsSelected = Children[SelectedTab] == Child;

			Child->Visible = IsSelected;
			if (Child->PanelElement)
				Child->PanelElement->IsVisible = Child->Visible;
			Child->UsedSize = UsedSize;
			Child->Position = Position;
			Child->UpdateLayout();
		}
	}
}

void engine::editor::EditorPanel::UpdatePanel()
{
	if (ShouldUpdate)
	{
		UpdateLayout();
		ShouldUpdate = false;
	}

	Update();

	for (EditorPanel* Child : Children)
	{
		Child->Update();
	}
}

void engine::editor::EditorPanel::OnResized()
{

}

void engine::editor::EditorPanel::Update()
{
}

engine::editor::EditorPanel* engine::editor::EditorPanel::SetWidth(float NewWidth)
{
	WidthFraction = NewWidth;
	return this;
}

void engine::editor::EditorPanel::AddChild(EditorPanel* NewChild, Align ChildAlign)
{
	if (ChildAlign == ChildrenAlign || Children.empty())
	{
		NewChild->Parent = this;
		Children.push_back(NewChild);
		ShouldUpdate = true;
		ChildrenAlign = ChildAlign;
	}
}

void engine::editor::EditorPanel::GenerateTabs()
{
	if (!PanelElement)
		return;

	PanelElement->tabBox->DeleteChildren();

	auto* ChildrenList = &Children;
	size_t Selected = SelectedTab;

	if (Parent && Parent->ChildrenAlign == Align::Tabs)
	{
		ChildrenList = &Parent->Children;
		Selected = Parent->SelectedTab;
	}
	else if (Parent && Children.empty())
	{
		AddTabFor(this, true);
		return;
	}
	for (EditorPanel* Child : *ChildrenList)
	{
		AddTabFor(Child, ChildrenList->at(Selected) == Child);
	}
}

void engine::editor::EditorPanel::AddTabFor(EditorPanel* Target, bool Selected)
{
	EditorPanelTab* NewTab = new EditorPanelTab();
	NewTab->SetSelected(Selected);
	NewTab->SetTitle(Target->Name);

	if (Target != this)
	{
		NewTab->mainButton->OnClicked = [this, Target]()
			{
				if (!Parent || Parent->ChildrenAlign != Align::Tabs)
				{
					return;
				}
				for (size_t i = 0; i < Parent->Children.size(); i++)
				{
					if (Parent->Children[i] == Target)
					{
						Parent->SelectedTab = i;
						Parent->ShouldUpdate = true;
						break;
					}
				}
			};
	}

	if (Selected)
	{
		NewTab->SetBorderColor(EditorUI::Theme.Highlight1);
		NewTab->SetColor(EditorUI::Theme.HighlightDark);
		NewTab->SetBorderSize(1);
		NewTab->SetPaddingSize(-1);
	}
	else
	{
		NewTab->SetColor(EditorUI::Theme.DarkBackground);
		NewTab->SetBorderSize(0);
		NewTab->SetPaddingSize(0);
	}
	PanelElement->tabBox->AddChild(NewTab);
}

kui::Vec2f engine::editor::EditorPanel::UsedSizeToPanelSize(kui::Vec2f Used)
{
	return Used - UIBox::PixelSizeToScreenSize(Vec2f(PanelPadding * 2, PanelPadding * 2 + TabsSize), Window::GetActiveWindow());
}

kui::Vec2f engine::editor::EditorPanel::PositionToPanelPosition(kui::Vec2f Pos)
{
	return Pos + UIBox::PixelSizeToScreenSize(Vec2f(PanelPadding, PanelPadding), Window::GetActiveWindow());
}
#endif