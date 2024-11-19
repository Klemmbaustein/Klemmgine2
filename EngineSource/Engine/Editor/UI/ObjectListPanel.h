#pragma once
#include "EditorPanel.h"

namespace engine::editor
{
	class ObjectListPanel : public EditorPanel
	{
	public:
		ObjectListPanel();

		virtual void Update() override;
	};
}