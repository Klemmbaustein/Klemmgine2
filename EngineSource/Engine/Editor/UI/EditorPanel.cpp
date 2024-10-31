#ifdef EDITOR
#include "EditorPanel.h"
#include <kui/Window.h>
#include "EditorUI.h"
#include <iostream>
using namespace kui;

const float PanelPadding = 5;
const float TabsSize = 25;

engine::editor::EditorPanel::EditorPanel(string Name, string InternalName)
{
	PanelElement = new EditorPanelElement();

	ShouldUpdate = true;
}

void engine::editor::EditorPanel::UpdateLayout()
{
	if (!Parent)
	{
		UsedSize = EditorUI::Instance->MainBackground->GetMinSize();
		Position = EditorUI::Instance->MainBackground->GetPosition();
	}

	PanelElement->SetSize(UsedSizeToPanelSize(UsedSize));
	PanelElement->SetPosition(PositionToPanelPosition(Position));
}

void engine::editor::EditorPanel::Update()
{
	if (ShouldUpdate)
	{
		UpdateLayout();
	}
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