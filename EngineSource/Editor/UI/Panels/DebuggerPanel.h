#pragma once
#include <Editor/UI/Panels/EditorPanel.h>
#include <ds/debug/debugState.hpp>
#include <kui/UI/UIScrollBox.h>
#include <Editor/UI/Debugger/ValueVisualizer.h>

namespace engine::editor
{
	class DebuggerPanel : public EditorPanel
	{
	public:

		DebuggerPanel();

		void Update() override;
		void OnResized() override;
		void OnThemeChanged() override;

		void UpdateFromState(ds::DebugState* State);

		void ShowFrame(ds::DebugFrame* Frame);

		void ShowVariableList(std::vector<ds::DebugVariable> Variables, kui::UIBox* ToBox, size_t Depth);

	private:
		void NavigateTo(ds::DebugFrame* Frame);

		kui::UIBackground* Separator = nullptr;

		kui::UIScrollBox* CallStackBackground = nullptr;
		kui::UIScrollBox* FrameBackground = nullptr;
		ds::DebugState* LastState = nullptr;
		ds::DebugFrame* SelectedFrame = nullptr;

		std::vector<ValueVisualizer*> Visualizers;

		std::map<ds::TypeId, ValueVisualizer*> VisualizerMap;

		kui::UISize FrameBackgroundSize = 0;

		bool IsHorizontal = true;

		ValueVisualizer* GetVisualizerForType(const ds::DebugVariable& Variable);
	};
}