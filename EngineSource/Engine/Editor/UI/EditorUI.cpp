#ifdef EDITOR
#include "EditorUI.h"
#include <Engine/Engine.h>
#include <Engine/Subsystem/VideoSubsystem.h>
#include <EditorPanel.kui.hpp>

const float StatusBarSize = 24;
const float MenuBarSize = 30;
engine::editor::EditorUI* engine::editor::EditorUI::Instance = nullptr;
engine::editor::EditorUI::EditorUI()
{
	using namespace kui;
	using namespace subsystem;

	Instance = this;

	UIBackground* Root = new UIBackground(false, -1, 0.1f, 2);
	Root
		->SetMinSize(2)
		->SetMaxSize(2);

	MenuBar = new UIBackground(true, 0, 0.2f, MenuBarSize);
	MenuBar
		->SetSizeMode(UIBox::SizeMode::PixelRelative)
		->SetTryFill(true);
	Root->AddChild(MenuBar);

	MainBackground = new UIBox(true, 0);

	Root->AddChild(MainBackground);

	StatusBar = new UIBackground(true, 0, 0.2f, StatusBarSize);
	StatusBar
		->SetSizeMode(UIBox::SizeMode::PixelRelative)
		->SetTryFill(true);
	Root->AddChild(StatusBar);

	Root->UpdateElement();

	RootPanel = new EditorPanel("root");
}

void engine::editor::EditorUI::Update()
{
	using namespace kui;
	
	MainBackground->SetMinSize(Vec2f(
		2,
		2 - UIBox::PixelSizeToScreenSize(Vec2f(0, StatusBarSize + MenuBarSize), MainBackground->GetParentWindow()).Y)
	);

	RootPanel->Update();
}

#endif