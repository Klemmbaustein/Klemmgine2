#include "ScriptEditorWindow.h"
#include <Editor/UI/EditorUI.h>
#include <Editor/Settings/EditorSettings.h>
#include <Engine/MainThread.h>
#include <Editor/UI/Panels/ScriptEditorPanel.h>
#include <Editor/UI/Panels/Viewport.h>

using namespace kui;
using namespace engine::editor;

ScriptEditorWindow* ScriptEditorWindow::Current = nullptr;

engine::editor::ScriptEditorWindow::ScriptEditorWindow()
	: IPopupWindow("Scripts", Vec2ui(600, 400), true, true)
{
	if (EditorUI::Instance->ScriptEditorWindowOpen)
	{
		delete this;
		return;
	}

	EditorUI::Instance->ScriptEditorWindowOpen = true;
	this->Open();
}

void engine::editor::ScriptEditorWindow::Begin()
{
	this->MonospacedFont = new Font(EditorUI::Asset("EditorMono.ttf"));
	Background = new UIBackground(true, -1, EditorUI::Theme.Background);

	Background->SetSize(2);

	UI = new ScriptEditorUI(Background, true, DefaultFont, MonospacedFont, Queue);
	UI->OnResized(2);

	Popup->OnResizedCallback = [this](Window*) {
		UI->OnResized(2);
	};
	Current = this;
}

void engine::editor::ScriptEditorWindow::Update()
{
	DropdownMenu::UpdateDropdowns();
	UI->Update();
	Queue->Update();
}

void engine::editor::ScriptEditorWindow::Destroy()
{
	Current = nullptr;
	Settings::GetInstance()->Interface.RemoveListener(this);
	delete MonospacedFont;
	delete UI;
	EditorUI::Instance->ScriptEditorWindowOpen = false;

	thread::ExecuteOnMainThread([] {
		bool Found = false;

		EditorUI::ForEachPanel<ScriptEditorPanel>([&Found](ScriptEditorPanel*) {
			Found = true;
		});

		if (Found)
		{
			return;
		}

		auto New = new ScriptEditorPanel();
		Viewport::Current->AddChild(New, EditorPanel::Align::Tabs);
	});
}

void engine::editor::ScriptEditorWindow::OnThemeChanged()
{
	Background->SetColor(EditorUI::Theme.Background);
	UI->UpdateColors();
}
