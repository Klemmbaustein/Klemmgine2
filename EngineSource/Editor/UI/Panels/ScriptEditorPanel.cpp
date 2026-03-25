#include "ScriptEditorPanel.h"
#include <Editor/UI/EditorUI.h>
#include <Engine/MainThread.h>
#include <Editor/UI/Windows/ScriptEditorWindow.h>

using namespace kui;
using namespace engine::editor;
using namespace engine;

engine::editor::ScriptEditorPanel::ScriptEditorPanel()
	: EditorPanel("Scripts", "ScriptEditorPanel"),
	UI(Background, false, EditorUI::EditorFont, EditorUI::MonospaceFont, thread::MainThreadQueue)
{
	UI.ScriptToolbar->AddButton("Float window", EditorUI::Asset("Open.png"), [this]() {
		delete this;
		new ScriptEditorWindow();
	});
}

engine::editor::ScriptEditorPanel::~ScriptEditorPanel()
{
}

void engine::editor::ScriptEditorPanel::OnResized()
{
	UI.OnResized(Size);
}

void engine::editor::ScriptEditorPanel::Update()
{
	UI.HasFocus = EditorUI::FocusedPanel == this;
	UI.IsVisible = Visible;
	UI.Update();
	this->AdditionalBoxes = UI.GetEditorBoxes();
}

void engine::editor::ScriptEditorPanel::OnThemeChanged()
{
	UI.UpdateColors();
}
