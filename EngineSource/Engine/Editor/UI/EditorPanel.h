#ifdef EDITOR
#pragma once
#include <Engine/Types.h>
#include <kui/Vec2.h>
#include <EditorPanel.kui.hpp>

namespace engine::editor
{
	class EditorPanel
	{
	public:
		EditorPanel(string Name, string InternalName = "panel");

		kui::Vec2f Size;
		kui::Vec2f PanelPosition;
		EditorPanel* Parent = nullptr;
		std::vector<EditorPanel*> Children;
		EditorPanelElement* PanelElement = nullptr;

		void UpdateLayout();

		void Update();
		bool ShouldUpdate = false;

	private:
		kui::Vec2f UsedSize;
		kui::Vec2f Position;
		kui::Vec2f UsedSizeToPanelSize(kui::Vec2f Used);
		kui::Vec2f PositionToPanelPosition(kui::Vec2f Pos);
	};
}
#endif