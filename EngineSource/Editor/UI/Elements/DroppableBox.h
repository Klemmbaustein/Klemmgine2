#pragma once
#include <kui/UI/UIBox.h>
#include <Editor/UI/EditorUI.h>

namespace engine::editor
{
	class DroppableBox : public kui::UIBox
	{
	public:

		using OnDropFn = std::function<void(EditorUI::DraggedItem Item)>;

		thread_local static std::vector<DroppableBox*> CurrentBoxes;

		DroppableBox(bool IsHorizontal, OnDropFn OnDrop);
		virtual ~DroppableBox() override;

		static DroppableBox* GetBoxAtCursor();

		OnDropFn OnDrop;
	};
}