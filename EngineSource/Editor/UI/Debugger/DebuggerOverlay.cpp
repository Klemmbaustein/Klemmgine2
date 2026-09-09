#include "DebuggerOverlay.h"
#include <Editor/EditorSubsystem.h>
#include <Engine/Engine.h>
#include <Engine/Graphics/VideoSubsystem.h>
#include <Engine/MainThread.h>
#include <Engine/Subsystem/InputSubsystem.h>
#include <Editor/UI/Panels/DebuggerPanel.h>
#include <Editor/UI/Panels/ConsolePanel.h>
#include <Editor/Settings/EditorSettings.h>

using namespace kui;

engine::editor::DebuggerOverlay::DebuggerOverlay(script::ScriptSubsystem* Scripts)
{
	UIBox* RootBox = new UIBox(false, -1);
	RootBox->SetSize(2);

	auto ToolbarBox = new UIBox(true, 0);

	ToolbarBox->SetSize(SizeVec::Pixels(400, 40));

	DebuggerToolbar = new Toolbar(false, EditorUI::Theme.HighlightDark);
	DebuggerToolbar->SetBorderEdges(false, true, true, true);
	DebuggerToolbar->SetBorderColor(EditorUI::Theme.Highlight1);
	DebuggerToolbar->SetCorner(EditorUI::Theme.CornerSize);
	DebuggerToolbar->SetCorners(false, false, true, true);
	DebuggerToolbar->AddButton("Continue", EditorUI::Asset("Run.png"), [Scripts] {
		Scripts->IsOnBreakpoint = false;
	});
	DebuggerToolbar->AddButton("Stop", EditorUI::Asset("X.png"), [Scripts] {
		Scripts->IsOnBreakpoint = false;
		Scripts->StopAfterBreakpoint = true;
	});
	DebuggerToolbar->AddButton("Open Debugger", EditorUI::Asset("Open.png"), [Scripts] {
		bool Found = false;

		EditorUI::ForEachPanel<DebuggerPanel>([&Found](DebuggerPanel* i) {
			i->SetFocused();
			Found = true;
		});

		if (!Found)
		{
			EditorUI::ForEachPanel<ConsolePanel>([&Found](ConsolePanel* i) {
				if (!Found)
				{
					i->AddChild(new DebuggerPanel(), EditorPanel::Align::Tabs, true);
				}
				Found = true;
			});
		}
	});

	Settings::GetInstance()->Interface.ListenToSetting(this, "theme", [this](SerializedValue) {
		DebuggerToolbar->SetBorderColor(EditorUI::Theme.Highlight1);
		DebuggerToolbar->SetToolbarColor(EditorUI::Theme.HighlightDark);
		DebuggerToolbar->SetCorner(EditorUI::Theme.CornerSize);
	});

	RootBox->SetHorizontalAlign(UIBox::Align::Centered);

	RootBox->AddChild(ToolbarBox);
	ToolbarBox->AddChild(DebuggerToolbar);
	RootBox->UpdateElement();
	ToolbarBox->RedrawElement();

	AddShortcut(Key::F5, ShortcutModifiers{}, [Scripts] {
		Scripts->IsOnBreakpoint = false;
	}, ShortcutOptions::AllowInText);

	input::ShowMouseCursor = true;
	Engine::GameHasFocus = false;
	Engine::IsPaused = true;
}

engine::editor::DebuggerOverlay::~DebuggerOverlay()
{
	Settings::GetInstance()->Interface.RemoveListener(this);
	delete DebuggerToolbar;
	Engine::IsPaused = false;
}

void engine::editor::DebuggerOverlay::Update()
{
	auto& w = VideoSubsystem::Current->MainWindow;

	auto Input = Engine::GetSubsystem<subsystem::InputSubsystem>();

	thread::MainThreadUpdate();
	Input->Update();
	Engine::GetSubsystem<editor::EditorSubsystem>()->Update();
	VideoSubsystem::Current->Update();
	VideoSubsystem::Current->RenderUpdate();

	DebuggerToolbar->GetAbsoluteParent()->MoveToFront();
}

bool engine::editor::DebuggerOverlay::HasKeyboardFocus()
{
	return true;
}