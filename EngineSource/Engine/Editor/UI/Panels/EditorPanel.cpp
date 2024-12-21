#ifdef EDITOR
#include "EditorPanel.h"
#include <kui/Window.h>
#include <kui/UI/UIBlurBackground.h>
#include <Engine/Editor/UI/EditorUI.h>
#include <cmath>
#include <Engine/Log.h>
#include <Engine/Error/EngineError.h>
using namespace kui;
using namespace engine::editor;

const float PANEL_PADDING = 3;
const float TABS_SIZE = 25;

EditorPanel* EditorPanel::DraggedPanel = nullptr;
bool EditorPanel::DraggingHorizontal = false;
float EditorPanel::DragStartPosition = 0;
kui::UIBackground* EditorPanel::PanelMoveHighlight = nullptr;
EditorPanel* EditorPanel::MovedPanel = nullptr;
EditorPanel* EditorPanel::MoveEndPanel = nullptr;
engine::editor::EditorPanel::EditorPanel(string Name, string InternalName)
{
	TypeName = InternalName;
	if (InternalName != "panel")
	{
		PanelElement = new EditorPanelElement();
		Background = PanelElement->panelBackground;
		Background->HasMouseCollision = true;
	}
	ShouldUpdate = true;
	this->Name = Name;
}

engine::editor::EditorPanel::~EditorPanel()
{
	if (EditorUI::FocusedPanel == this)
	{
		EditorUI::FocusedPanel = nullptr;
	}

	delete PanelElement;

	ClearParent();
}

void engine::editor::EditorPanel::UpdateLayout()
{
	if (!Parent)
	{
		UsedSize = EditorUI::Instance->MainBackground->GetMinSize();
		Position = EditorUI::Instance->MainBackground->GetPosition();
	}

	if (PanelElement)
	{
		UpdateFocusState();
		GenerateTabs();
		Size = UsedSizeToPanelSize(UsedSize);
		PanelElement->SetSize(Size);
		PanelPosition = PositionToPanelPosition(Position);
		PanelElement->SetPosition(PanelPosition);
	}

	if (ChildrenAlign != Align::Tabs)
	{
		float Pos = 0;
		float CurrentFraction = 0;
		float MinFraction = 0.1f;
		size_t it = 0;
		for (EditorPanel* Child : Children)
		{
			float PossibleFraction = std::max(Child->SizeFraction, 0.1f);
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
	OnResized();
}

void engine::editor::EditorPanel::UpdatePanel()
{
	if (ShouldUpdate)
	{
		UpdateLayout();
		ShouldUpdate = false;
	}

	if (PanelElement)
	{
		if (Visible)
		{
			bool IsFocused = EditorUI::FocusedPanel == this;
			auto HoveredBox = PanelElement->GetParentWindow()->UI.HoveredBox;
			if (!IsFocused && HoveredBox && PanelElement->GetParentWindow()->Input.IsLMBClicked && HoveredBox->IsChildOf(PanelElement))
			{
				SetFocused();
			}
			UpdatePanelMove();
		}

		Update();
	}
	HandleResizing();


	for (EditorPanel* Child : Children)
	{
		Child->UpdatePanel();
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
	SizeFraction = NewWidth;
	return this;
}

void engine::editor::EditorPanel::UpdateAllPanels()
{
	if (MoveEndPanel)
	{
		if (MoveEndPanel != MovedPanel)
		{
			MovedPanel->ClearParent();
			MoveEndPanel->AddChild(MovedPanel, Align::Tabs, true);
		}
		MovedPanel = nullptr;
		MoveEndPanel = nullptr;
		delete PanelMoveHighlight;
		PanelMoveHighlight = nullptr;
	}
}

void engine::editor::EditorPanel::AddChild(EditorPanel* NewChild, Align ChildAlign, bool Select)
{
	if ((ChildAlign == this->ChildrenAlign || Children.empty()) && TypeName == "panel")
	{
		NewChild->Parent = this;
		Children.push_back(NewChild);
		ShouldUpdate = true;
		this->ChildrenAlign = ChildAlign;
		if (Select)
		{
			SelectedTab = Children.size() - 1;
			NewChild->SetFocused();
		}
	}
	else if (TypeName != "panel" && Parent)
	{
		if (Parent->ChildrenAlign != ChildAlign)
		{
			EditorPanel* New = new EditorPanel("panel");
			New->Children = { this, NewChild };
			New->Parent = this->Parent;
			New->ChildrenAlign = ChildAlign;

			for (EditorPanel*& Child : Parent->Children)
			{
				if (Child == this)
				{
					Child = New;
				}
			}
			if (Select)
			{
				New->SelectedTab = 1;
				NewChild->SetFocused();
			}
			Parent->ShouldUpdate = true;

			NewChild->Parent = New;
			this->Parent = New;
		}
		else
		{
			Parent->AddChild(NewChild, ChildAlign, Select);
		}
	}
	else
		ENGINE_UNREACHABLE();
}

void engine::editor::EditorPanel::GenerateTabs()
{
	if (!PanelElement)
		return;

	PanelElement->tabBox->DeleteChildren();
	TabElements.clear();

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

void engine::editor::EditorPanel::SetFocused()
{
	if (EditorUI::FocusedPanel == this)
		return;

	EditorPanel* Old = EditorUI::FocusedPanel;
	EditorUI::FocusedPanel = this;
	if (Old)
	{
		Old->UpdateFocusState();
		Old->PanelElement->RedrawElement();
	}
	PanelElement->RedrawElement();
	UpdateFocusState();
}

void engine::editor::EditorPanel::ClearParent()
{
	if (!Parent)
	{
		return;
	}

	for (size_t i = 0; i < Parent->Children.size(); i++)
	{
		if (Parent->Children[i] != this)
		{
			continue;
		}

		if (Parent->ChildrenAlign == Align::Tabs && Parent->SelectedTab >= i && Parent->SelectedTab > 0)
		{
			Parent->SelectedTab--;
		}

		Parent->ShouldUpdate = true;
		Parent->Children.erase(Parent->Children.begin() + i);
		break;
	}

	if (Parent->Children.empty())
	{
		delete Parent;
	}
	Parent = nullptr;
}

void engine::editor::EditorPanel::HandleResizing()
{
	if (EditorUI::Instance->DraggedBox)
		return;
	if (DraggedPanel)
	{
		HandleResizeDrag();
		return;
	}
	Window* Win = Window::GetActiveWindow();
	Vec2f PixelSize = UIBox::PixelSizeToScreenSize(PANEL_PADDING, Win);
	Vec2f MousePos = Win->Input.MousePosition;

	bool Hovering = false;
	bool HoverHorizontal = false;

	auto IsBetween = [](float Value, float Min, float Offset)
		{
			return Value > Min && Value < Min + Offset;
		};

	if (std::abs(MousePos.X - (Position.X + UsedSize.X)) < PixelSize.X
		&& IsBetween(MousePos.Y, Position.Y, UsedSize.Y))
	{
		Hovering = true;
		HoverHorizontal = true;
	}
	else if (std::abs(MousePos.Y - (Position.Y + UsedSize.Y)) < PixelSize.Y
		&& IsBetween(MousePos.X, Position.X, UsedSize.X))
	{
		Hovering = true;
		HoverHorizontal = false;
	}

	if (!Hovering)
		return;

	if (!Parent || Parent->ChildrenAlign != (HoverHorizontal ? Align::Horizontal : Align::Vertical))
		return;

	Win->CurrentCursor = HoverHorizontal ? Window::Cursor::ResizeLeftRight : Window::Cursor::ResizeUpDown;

	if (Hovering && Win->Input.IsLMBClicked)
	{
		DraggedPanel = this;
		DraggingHorizontal = HoverHorizontal;
		DragStartPosition = HoverHorizontal ? MousePos.X : MousePos.Y;
	}
}

void engine::editor::EditorPanel::HandleResizeDrag()
{
	if (this != DraggedPanel)
	{
		return;
	}

	Window* Win = Window::GetActiveWindow();
	Vec2f MousePos = Win->Input.MousePosition;

	if (!Win->Input.IsLMBDown)
	{
		float NewPos = DraggingHorizontal ? MousePos.X : MousePos.Y;
		float ParentSize = DraggingHorizontal ? Parent->UsedSize.X : Parent->UsedSize.Y;

		float NewSizeFraction = SizeFraction * ParentSize;
		NewSizeFraction += NewPos - DragStartPosition;
		NewSizeFraction /= ParentSize;

		SizeFraction = NewSizeFraction;
		Parent->ShouldUpdate = true;

		DraggedPanel = nullptr;
		return;
	}
	Win->CurrentCursor = DraggingHorizontal ? Window::Cursor::ResizeLeftRight : Window::Cursor::ResizeUpDown;
}

void engine::editor::EditorPanel::UpdateFocusState()
{
	bool Focused = this == EditorUI::FocusedPanel;
	PanelElement->SetBorderColor(Focused ? EditorUI::Theme.Highlight1 : EditorUI::Theme.BackgroundHighlight);

	size_t Selected = SelectedTab;
	auto* ChildrenList = &Children;

	if (Parent && Parent->ChildrenAlign == Align::Tabs)
	{
		Selected = Parent->SelectedTab;
	}

	if (TabElements.size() <= Selected)
		return;

	TabElements[Selected]->SetBorderColor(Focused ? EditorUI::Theme.Highlight1 : EditorUI::Theme.BackgroundHighlight);
	TabElements[Selected]->SetColor(Focused ? EditorUI::Theme.HighlightDark : EditorUI::Theme.Background);
}

void engine::editor::EditorPanel::MovePanel()
{
	if (EditorUI::Instance->DraggedBox)
		return;

	if (PanelMoveHighlight)
		delete PanelMoveHighlight;

	PanelMoveHighlight = new UIBlurBackground(true, 0, EditorUI::Theme.HighlightDark);
	PanelMoveHighlight
		->SetOpacity(0.5f)
		->SetBorder(1, UIBox::SizeMode::PixelRelative)
		->SetBorderColor(EditorUI::Theme.Highlight1);

	auto MoveBox = new DraggedPanelTab();

	bool Focused = EditorUI::FocusedPanel == this;

	MoveBox->SetTitle(Name);
	MovedPanel = this;

	EditorUI::Instance->DraggedBox = MoveBox;
}

void engine::editor::EditorPanel::UpdatePanelMove()
{
	if (!MovedPanel || MoveEndPanel)
		return;

	if (!PanelElement->IsBeingHovered())
		return;

	if (!Window::GetActiveWindow()->Input.IsLMBDown)
	{
		MoveEndPanel = this;
		return;
	}
	
	if (PanelMoveHighlight->GetPosition() != PanelPosition)
	{
		PanelMoveHighlight->SetPosition(PanelPosition);
		PanelMoveHighlight->SetMinSize(Size);
		PanelElement->RedrawElement();
	}
}

void engine::editor::EditorPanel::AddTabFor(EditorPanel* Target, bool Selected)
{
	EditorPanelTab* NewTab = new EditorPanelTab();
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
						Parent->Children[i]->SetFocused();
						break;
					}
				}
			};

		NewTab->mainButton->OnDragged = [Target](int)
			{
				Target->MovePanel();
			};
	}

	NewTab->closeButton->OnClicked = [Target]()
		{
			delete Target;
		};

	NewTab->mainButton->OnDragged = [Target](int)
		{
			Target->MovePanel();
		};

	if (Selected)
	{
		bool Focused = EditorUI::FocusedPanel == this;

		NewTab->SetBorderColor(Focused ? EditorUI::Theme.Highlight1 : EditorUI::Theme.BackgroundHighlight);
		NewTab->SetColor(Focused ? EditorUI::Theme.HighlightDark : EditorUI::Theme.Background);
		NewTab->SetBorderSize(1);
		NewTab->SetPaddingSize(-1);
	}
	else
	{
		NewTab->SetColor(EditorUI::Theme.DarkBackground2);
		NewTab->SetBorderSize(0);
		NewTab->SetPaddingSize(0);
	}

	if (!Target->CanClose)
		delete NewTab->closeButton;
	PanelElement->tabBox->AddChild(NewTab);
	TabElements.push_back(NewTab);
}

kui::Vec2f engine::editor::EditorPanel::UsedSizeToPanelSize(kui::Vec2f Used)
{
	return Used - UIBox::PixelSizeToScreenSize(Vec2f(PANEL_PADDING * 2, PANEL_PADDING * 2 + TABS_SIZE), Window::GetActiveWindow());
}

kui::Vec2f engine::editor::EditorPanel::PositionToPanelPosition(kui::Vec2f Pos)
{
	return Pos + UIBox::PixelSizeToScreenSize(Vec2f(PANEL_PADDING, PANEL_PADDING), Window::GetActiveWindow());
}
#endif