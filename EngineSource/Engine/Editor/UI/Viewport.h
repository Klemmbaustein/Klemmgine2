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
		bool MouseGrabbed = false;

		kui::Timer StatsRedrawTimer;
		bool RedrawStats = false;
		uint64 FameCount = 0;

		static Viewport* Current;

		void OnResized() override;
		void Update() override;
		kui::UIBackground* ViewportBackground = nullptr;

	private:

		kui::UIBackground* ViewportToolbar = nullptr;
		kui::UIText* ViewportStatusText = nullptr;
		kui::UIBox* StatusBarBox = nullptr;
		kui::UIBackground* LoadingScreenBox = nullptr;
	};
}
#endif