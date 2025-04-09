#ifdef EDITOR
#include "EditorPanel.h"
#include <kui/Window.h>
#include <kui/UI/UIBlurBackground.h>
#include <kui/UI/UIScrollBox.h>
#include <Editor/UI/EditorUI.h>
#include <cmath>
#include <Core/Log.h>
#include <Core/Error/EngineError.h>
using namespace kui;
using namespace engine::editor;

const float PANEL_PADDING = 3;
const float TABS_SIZE = 25;

EditorPanel* EditorPanel::DraggedPanel = nullptr;
bool EditorPanel::DraggingHorizontal = false;
float EditorPanel::DragStartPosition = 0;
EditorPanel::MoveOperation EditorPanel::Move;

static float IsBetween(float Value, float Min, float Offset)
{
	return Value > Min && Value < Min + Offset;
};

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

	for (auto& i : Children)
	{
		i->Parent = nullptr;
		delete i;
	}
}

void engine::editor::EditorPanel::UpdateLayout()
{
	if (!Parent)
	{
		UsedSize = EditorUI::Instance->MainBackground->GetMinSize().GetScreen();
		Position = EditorUI::Instance->MainBackground->GetPosition();
	}

	if (PanelElement)
	{
		UpdateFocusState();
		GenerateTabs();
		Size = UsedSizeToPanelSize(UsedSize);
		PanelElement->SetSize(SizeVec(Size));
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

			Child->SizeFraction = PossibleFraction;

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
			Child->Visible = true;
			if (Child->PanelElement)
				Child->PanelElement->IsVisible = Child->Visible;
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
	ShouldUpdate = false;

	OnResized();
}

void engine::editor::EditorPanel::UpdatePanel()
{
	if (ShouldUpdate)
	{
		UpdateLayout();
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

void engine::editor::EditorPanel::AddChild(EditorPanel* NewChild, Align ChildAlign, bool Select, size_t Position)
{
	ENGINE_ASSERT(NewChild != this);

	if ((ChildAlign == this->ChildrenAlign || Children.empty()) && TypeName == "panel")
	{
		NewChild->ClearParent();
		NewChild->Parent = this;
		ShouldUpdate = true;
		this->ChildrenAlign = ChildAlign;

		if (Position >= Children.size())
		{
			Children.push_back(NewChild);
			if (Select)
			{
				SelectedTab = Children.size() - 1;
				NewChild->SetFocused();
			}
		}
		else
		{
			Children.insert(Children.begin() + Position, NewChild);
			if (Select)
			{
				SelectedTab = Position;
				NewChild->SetFocused();
			}
		}
	}
	else if ((TypeName != "panel" || (this->ChildrenAlign == Align::Tabs && ChildAlign != Align::Tabs)) && Parent)
	{
		NewChild->ClearParent();
		if (Parent->ChildrenAlign != ChildAlign)
		{
			EditorPanel* New = new EditorPanel("panel");
			if (Position > 0)
				New->Children = { this, NewChild };
			else
				New->Children = { NewChild, this };

			New->Parent = this->Parent;
			New->ChildrenAlign = ChildAlign;
			New->SizeFraction = SizeFraction;
			this->SizeFraction = 0.5;
			NewChild->SizeFraction = 0.5;

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
			NewChild->SizeFraction = SizeFraction / 2;
			this->SizeFraction = SizeFraction / 2;
			Parent->AddChild(NewChild, ChildAlign, Select, Position);
		}
	}
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
	Window::GetActiveWindow()->Input.KeyboardFocusTargetBox = Background;
	if (Old)
	{
		Old->UpdateFocusState();
		Old->PanelElement->RedrawElement();
	}
	PanelElement->RedrawElement();
	UpdateFocusState();
}

void engine::editor::EditorPanel::SetName(string NewName)
{
	if (this->Name != NewName)
	{
		this->Name = NewName;
		if (Parent)
		{
			Parent->ShouldUpdate = true;
		}
		else
		{
			ShouldUpdate = true;
		}
	}
}

bool engine::editor::EditorPanel::OnClosed()
{
	return true;
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

	if (Win->UI.HoveredBox || UIScrollBox::IsDraggingScrollBox)
		return;

	Vec2f PixelSize = UIBox::PixelSizeToScreenSize(PANEL_PADDING, Win);
	Vec2f MousePos = Win->Input.MousePosition;

	bool Hovering = false;
	bool HoverHorizontal = false;


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

		float Difference = (NewPos - DragStartPosition) / ParentSize;
		SizeFraction += Difference;

		for (int64 i = 0; i < int64(Parent->Children.size() - 1); i++)
		{
			if (Parent->Children[i] == this)
			{
				Parent->Children[i + 1]->SizeFraction -= Difference;
				break;
			}
		}

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
	if (EditorUI::Instance->DraggedBox || DraggedPanel)
		return;

	if (Move.HighlightBackground)
		delete Move.HighlightBackground;

	Move.HighlightBackground = new UIBlurBackground(true, 0, EditorUI::Theme.HighlightDark);
	Move.HighlightBackground
		->SetOpacity(0.5f)
		->SetBorder(UISize::Pixels(3), EditorUI::Theme.Highlight1)
		->SetCorner(UISize::Pixels(5))
		->SetHorizontalAlign(UIBox::Align::Centered)
		->SetVerticalAlign(UIBox::Align::Centered);

	Move.HighlightBackground->AddChild((new UIText(11_px, EditorUI::Theme.Text, Name, EditorUI::EditorFont))
		->SetPadding(2_px, 2_px, 10_px, 10_px));

	auto MoveBox = new DraggedPanelTab();

	bool Focused = EditorUI::FocusedPanel == this;

	MoveBox->SetTitle(Name);
	Move.Panel = this;

	EditorUI::Instance->DraggedBox = MoveBox;
	EditorUI::Instance->CurrentDraggedItem.Centered = false;
}

void engine::editor::EditorPanel::UpdatePanelMove()
{
	if (!Move.Panel || Move.EndTarget)
		return;

	if (!PanelElement->panelBackground->IsBeingHovered() && !PanelElement->tabBox->IsBeingHovered())
		return;

	Window* Win = Window::GetActiveWindow();
	Vec2f MousePos = Win->Input.MousePosition;

	Move.TabPosition = SIZE_MAX;

	Vec2f PreviewPosition = PanelPosition;
	Vec2f PreviewSize = Size;
	if (PanelElement->tabBox->IsBeingHovered())
	{
		Move.TabAlign = Align::Tabs;
		for (size_t i = 0; i < TabElements.size(); i++)
		{
			PreviewPosition = TabElements[i]->mainButton->GetPosition();
			PreviewSize = TabElements[i]->mainButton->GetUsedSize().GetScreen();
			if (TabElements[i]->GetPosition().X + TabElements[i]->GetUsedSize().GetScreen().X > MousePos.X)
			{
				Move.TabPosition = i;
				break;
			}
			PreviewPosition += Vec2f(TabElements[i]->mainButton->GetUsedSize().GetScreen().X, 0);
		}
	}
	else
	{
		Vec2f RelativePos = (MousePos - PanelPosition) / Size;

		Move.MoveType = MoveOperation::Center;
		if (RelativePos.Y < 0.25)
		{
			Move.MoveType = MoveOperation::Down;
		}
		else if (RelativePos.Y > 0.75)
		{
			Move.MoveType = MoveOperation::Up;
		}
		else if (RelativePos.X < 0.25)
		{
			Move.MoveType = MoveOperation::Left;
		}
		else if (RelativePos.X > 0.75)
		{
			Move.MoveType = MoveOperation::Right;
		}

		switch (Move.MoveType)
		{
		case MoveOperation::Up:
			PreviewPosition += Vec2f(0, PreviewSize.Y / 2);
			PreviewSize = PreviewSize / Vec2f(1, 2);
			Move.TabAlign = Align::Vertical;
			Move.TabPosition = SIZE_MAX;
			break;
		case MoveOperation::Down:
			PreviewSize = PreviewSize / Vec2f(1, 2);
			Move.TabAlign = Align::Vertical;
			Move.TabPosition = 0;
			break;
		case MoveOperation::Left:
			PreviewSize = PreviewSize / Vec2f(2, 1);
			Move.TabAlign = Align::Horizontal;
			Move.TabPosition = 0;
			break;
		case MoveOperation::Right:
			PreviewPosition += Vec2f(PreviewSize.X / 2, 0);
			PreviewSize = PreviewSize / Vec2f(2, 1);
			Move.TabAlign = Align::Horizontal;
			Move.TabPosition = SIZE_MAX;
			break;
		case MoveOperation::Center:
			Move.TabAlign = Align::Tabs;
			break;
		default:
			break;
		}
	}
	Vec2f PixelSize = UIBox::PixelSizeToScreenSize(1, Window::GetActiveWindow());

	Move.HighlightBackground->IsVisible = true;
	
	PreviewPosition += PixelSize * Vec2f(2);
	PreviewSize = PreviewSize - PixelSize * Vec2f(4);
	if (!Window::GetActiveWindow()->Input.IsLMBDown)
	{
		if (Move.Panel == this)
			Move.EndTarget = Parent;
		else
			Move.EndTarget = this;
		return;
	}

	if (Move.HighlightBackground->GetPosition() != PreviewPosition
		|| Move.HighlightBackground->GetMinSize() != PreviewSize)
	{
		Move.HighlightBackground->SetPosition(PreviewPosition);
		Move.HighlightBackground->SetMinSize(PreviewSize);
		PanelElement->RedrawElement();
	}
}

void engine::editor::EditorPanel::UpdateAllPanels()
{
	if (Move.HighlightBackground)
	{
		Move.HighlightBackground->IsVisible = false;
	}
	if (Move.EndTarget)
	{
		Move.HighlightBackground->IsVisible = true;
		if (Move.EndTarget != Move.Panel->Parent
			|| Move.EndTarget->Children.size() != 1
			|| Move.EndTarget->ChildrenAlign != Align::Tabs)
		{
			Move.EndTarget->AddChild(Move.Panel, Move.TabAlign, true, Move.TabPosition);
		}
		Move.Panel = nullptr;
		Move.EndTarget = nullptr;
		delete Move.HighlightBackground;
		Move.HighlightBackground = nullptr;
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
			if (Target->OnClosed())
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
		NewTab->SetBorderSize(1_px);
		NewTab->SetPaddingSize(UISize::Pixels(-1));
	}
	else
	{
		NewTab->SetColor(EditorUI::Theme.DarkBackground2);
		NewTab->SetBorderSize(0_px);
		NewTab->SetPaddingSize(0_px);
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