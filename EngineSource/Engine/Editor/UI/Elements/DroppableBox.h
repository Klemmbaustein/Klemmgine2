#pragma once
#include <kui/UI/UIBox.h>
#include <Engine/Editor/UI/EditorUI.h>

namespace engine::editor
{
	class DroppableBox : public kui::UIBox
	{
	public:
		DroppableBox(bool IsHorizontal);

		std::function<void(EditorUI::DraggedItem Item)> OnDrop;
	};
}