#pragma once
#include "EditorPanel.h"

namespace engine::editor
{
	class MessagePanel : public EditorPanel
	{
	public:
		MessagePanel();

		virtual void Update() override;
	};
}