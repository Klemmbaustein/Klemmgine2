#include "DebuggerPanel.h"
#include <Engine/Script/ScriptSubsystem.h>
#include <Editor/UI/EditorUI.h>
#include <ds/debug/debugState.hpp>
#include <DebuggerPanel.kui.hpp>
#include <Editor/UI/Debugger/PrimitiveVisualizer.h>
#include <Editor/UI/Debugger/ClassVisualizer.h>
#include <algorithm>
#include <Editor/Assets/ScriptAssetType.h>

using namespace kui;
using namespace engine::script;
using namespace engine::editor;

engine::editor::DebuggerPanel::DebuggerPanel()
	: EditorPanel("Debugger", "DebuggerPanel")
{
	Visualizers = {
		new StringVisualizer(),
		new IntVisualizer(),
		new FloatVisualizer(),
		new BoolVisualizer(),
		new ClassVisualizer(),
	};

	std::sort(Visualizers.begin(), Visualizers.end(), [](ValueVisualizer* a, ValueVisualizer* b) {
		return a->Priority > b->Priority;
	});

	Background->SetHorizontal(true);
	CallStackBackground = new UIScrollBox(false, 0, true);

	Background->AddChild(CallStackBackground);

	Separator = new UIBackground(true, 0, EditorUI::Theme.BackgroundHighlight, SizeVec(1_px, UISize::Parent(1)));
	Background->AddChild(Separator
		->SetPadding(1_px));

	FrameBackground = new UIScrollBox(false, 0, true);

	Background->AddChild(FrameBackground
		->SetPadding(3_px));
}

void engine::editor::DebuggerPanel::Update()
{
	if (ScriptSubsystem::Instance->CurrentBreakpointState != LastState)
	{
		SelectedFrame = nullptr;
		UpdateFromState(ScriptSubsystem::Instance->CurrentBreakpointState);
		VisualizerMap.clear();
		if (LastState)
		{
			SetFocused();
			NavigateTo(SelectedFrame);
		}
		else
		{
			auto ScriptAsset = dynamic_cast<editor::ScriptAssetType*>(
				editor::EditorUI::Instance->GetAssetTypeForExtension("ds"));

			if (ScriptAsset)
			{
				ScriptAsset->RunOnActiveScriptEditor(
					[](editor::ScriptEditorUI* UI) {
					UI->ClearDebugHighlights();
				});
			}
		}
	}
}

void engine::editor::DebuggerPanel::OnResized()
{
	IsHorizontal = UISize(this->Size.X).GetPixels().X > 550;
	Background->SetHorizontal(IsHorizontal);
	Separator->SetMinSize(IsHorizontal ? SizeVec(1_px, UISize::Parent(1)) : SizeVec(UISize::Parent(1), 1_px));
	if (IsHorizontal)
	{
		FrameBackgroundSize = this->Size.X - (265_px).GetScreen().X;
		FrameBackground
			->SetMinWidth(FrameBackgroundSize)
			->SetMaxWidth(FrameBackgroundSize)
			->SetMinHeight(UISize::Parent(1))
			->SetMaxHeight(UISize::Parent(1));

		CallStackBackground
			->SetMinWidth(250_px)
			->SetPadding(3_px, 3_px, 3_px, 1_px);

		CallStackBackground
			->SetMinHeight(UISize::Parent(1))
			->SetMaxHeight(UISize::Parent(1));
	}
	else
	{
		FrameBackground
			->SetMinWidth(UISize::Parent(1))
			->SetMaxWidth(UISize::Parent(1));

		CallStackBackground
			->SetMinWidth(UISize::Parent(1))
			->SetMaxWidth(UISize::Parent(1));

		CallStackBackground
			->SetMinHeight(0)
			->SetPadding(3_px);
	}

	if (LastState)
	{
		UpdateFromState(LastState);
	}
}

void engine::editor::DebuggerPanel::OnThemeChanged()
{
	Separator->SetColor(EditorUI::Theme.BackgroundHighlight);
	UpdateFromState(LastState);
}

void engine::editor::DebuggerPanel::UpdateFromState(ds::DebugState* State)
{
	LastState = State;
	if (!State)
	{
		CallStackBackground->DeleteChildren();
		FrameBackground->DeleteChildren();
		return;
	}

	CallStackBackground->DeleteChildren();

	auto Frames = State->getFrames();

	auto& DebugData = ScriptSubsystem::Instance->ScriptInstructions->debug;

	if (!SelectedFrame)
	{
		SelectedFrame = Frames.empty() ? nullptr : Frames[0];
	}

	for (auto& i : Frames)
	{
		auto Item = new DebuggerCallStackEntry();

		Item->SetName(DebugData.getSectionAt(i->getOffset())->name);

		bool Selected = SelectedFrame == i;

		if (Selected)
		{
			Item->btn->SetBorder(1_px, EditorUI::Theme.Highlight1);
			Item->btn->SetColor(EditorUI::Theme.HighlightDark);
			Item->btn->SetHoveredColor(EditorUI::Theme.HighlightDark);
			Item->btn->SetKeyboardHoveredColor(EditorUI::Theme.HighlightDark);
			Item->btn->SetPressedColor(EditorUI::Theme.HighlightDark);
		}
		Item->btn->OnClicked = [this, i = i, State] {
			SelectedFrame = i;
			UpdateFromState(State);
			NavigateTo(SelectedFrame);
		};

		CallStackBackground->AddChild(Item);
	}

	if (!IsHorizontal)
	{
		CallStackBackground->UpdateElement();
		auto s = CallStackBackground->GetUsedSize().GetScreen().Y;
		FrameBackgroundSize = Size.Y - s - (15_px).GetScreen().Y;
		FrameBackground
			->SetMinHeight(FrameBackgroundSize)
			->SetMaxHeight(FrameBackgroundSize);
	}

	ShowFrame(SelectedFrame);
}

void engine::editor::DebuggerPanel::ShowFrame(ds::DebugFrame* Frame)
{
	FrameBackground->GetScrollObject()->Scrolled = 0;

	FrameBackground->DeleteChildren();
	FrameBackground->AddChild((new UIText(12_px, EditorUI::Theme.Text, "Local variables", EditorUI::EditorFont))
		->SetPadding(5_px));
	FrameBackground->AddChild((new UIBackground(false, 0, EditorUI::Theme.BackgroundHighlight, SizeVec(UISize::Parent(1), 1_px)))
		->SetPadding(1_px, 0_px, 3_px, 3_px));

	ShowVariableList(Frame->getVariables(), FrameBackground, 0);
}

void engine::editor::DebuggerPanel::ShowVariableList(std::vector<ds::DebugVariable> Variables, kui::UIBox* ToBox, size_t Depth)
{
	for (auto& i : Variables)
	{
		auto Item = new DebuggerVariableElement();

		Item->SetNameWidth(UISize::Pixels((IsHorizontal ? 200 : 100) - Depth * 16));
		if (IsHorizontal)
		{
			Item->SetValueWidth(UISize::Pixels(UISize(Size.X).GetPixels().X - 500));
		}
		else
		{
			Item->SetValueWidth(UISize::Pixels(UISize(Size.X).GetPixels().X - 150));
		}

		Item->SetName(i.name);

		string ValueString = "<Unknown>";
		std::vector<ds::DebugVariable> Members;

		auto Visualizer = GetVisualizerForType(i);
		if (Visualizer)
		{
			ValueString = Visualizer->GetPreviewText(i);
			Members = Visualizer->GetMembers(i);
		}
		Item->SetValueText(ValueString);

		if (!Members.empty())
		{
			Item->btn->OnClicked = [this, Item, Members, Depth = Depth + 1] {
				if (Item->children->GetChildren().empty())
				{
					ShowVariableList(Members, Item->children, Depth);
					Item->SetCollapseIcon(EditorUI::Asset("DownArrow.png"));
				}
				else
				{
					Item->children->DeleteChildren();
					Item->SetCollapseIcon(EditorUI::Asset("RightArrow.png"));
				}
			};
		}
		else
		{
			Item->collapse->IsVisible = false;
		}

		if (&*Variables.rbegin() == &i)
		{
			Item->nameText->SetDownPadding(8_px);
		}

		ToBox->AddChild(Item);
	}
}

void engine::editor::DebuggerPanel::NavigateTo(ds::DebugFrame* Frame)
{
	auto ScriptAsset = dynamic_cast<editor::ScriptAssetType*>(
		editor::EditorUI::Instance->GetAssetTypeForExtension("ds"));

	if (ScriptAsset)
	{
		auto [Section, Line] = script::ScriptSubsystem::Instance->ScriptInstructions->debug.getLineAt(Frame->getOffset());

		if (!Section || !Line)
		{
			return;
		}

		ScriptAsset->RunOnActiveScriptEditor(
			[File = Section->file, Line = Line->lineNumber](editor::ScriptEditorUI* UI) {
			UI->HighlightLine(File, Line);
		});
	}
}

ValueVisualizer* engine::editor::DebuggerPanel::GetVisualizerForType(const ds::DebugVariable& Variable)
{
	auto found = VisualizerMap.find(Variable.type);

	if (found != VisualizerMap.end())
	{
		return found->second;
	}

	for (auto& v : Visualizers)
	{
		if (v->SupportsType(Variable))
		{
			VisualizerMap[Variable.type] = v;
			return v;
		}
	}

	return nullptr;
}
