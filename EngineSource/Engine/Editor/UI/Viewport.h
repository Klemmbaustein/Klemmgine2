#ifdef EDITOR
#pragma once
#include "EditorPanel.h"
#include <kui/Timer.h>
#include <kui/UI/UIText.h>

namespace engine::editor
{
	class Viewport : public EditorPanel
	{
	public:
		Viewport();

		kui::UIBackground* ViewportBackground = nullptr;
		kui::UIText* ViewportStatusText = nullptr;
		kui::UIBox* StatusBarBox = nullptr;
		bool MouseGrabbed = false;

		kui::Timer StatsRedrawTimer;
		bool RedrawStats = false;
		uint64 FameCount = 0;

		void OnResized() override;
		void Update() override;
	};
}
#endif