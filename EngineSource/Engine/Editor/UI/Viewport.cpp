#ifdef EDITOR
#include "Viewport.h"
#include <Engine/Engine.h>
#include <Engine/Subsystem/SceneSubsystem.h>
#include <Engine/Subsystem/VideoSubsystem.h>
#include <Engine/Input.h>
#include <kui/UI/UISpinner.h>
#include "EditorUI.h"
#include <iostream>
#include <Toolbar.kui.hpp>
using namespace engine::subsystem;
using namespace kui;

const float TOOLBAR_SIZE = 38;

engine::editor::Viewport* engine::editor::Viewport::Current = nullptr;

engine::editor::Viewport::Viewport()
	: EditorPanel("Viewport", "viewport")
{
	VideoSubsystem* VideoSystem = Engine::GetSubsystem<VideoSubsystem>();
	
	kui::Shader* HoleShader = VideoSystem->MainWindow->Shaders.LoadShader("shaders/uishader.vert", "ui/ui_hole.frag", "ui hole shader");

	ViewportBackground = new UIBackground(false, 0, 1, 0, HoleShader);
	ViewportBackground
		->SetVerticalAlign(UIBox::Align::Centered)
		->SetHorizontalAlign(UIBox::Align::Centered)
		->SetPadding(0, 1, 1, 1)
		->SetPaddingSizeMode(UIBox::SizeMode::PixelRelative);
	ViewportBackground->HasMouseCollision = true;

	StatusBarBox = new UIBox(true, 0);
	StatusBarBox
		->SetMinSize(23)
		->SetMaxSize(23)
		->SetTryFill(true)
		->SetSizeMode(UIBox::SizeMode::PixelRelative)
		->SetVerticalAlign(UIBox::Align::Centered);

	ViewportToolbar = new UIBackground(true, 0, EditorUI::Theme.Background);
	ViewportToolbar
		->SetBorder(1, UIBox::SizeMode::PixelRelative)
		->SetBorderEdges(false, true, false, false)
		->SetBorderColor(EditorUI::Theme.BackgroundHighlight)
		->SetMinSize(TOOLBAR_SIZE)
		->SetMaxSize(TOOLBAR_SIZE)
		->SetTryFill(true)
		->SetPadding(1, 0, 1, 1)
		->SetPaddingSizeMode(UIBox::SizeMode::PixelRelative)
		->SetSizeMode(UIBox::SizeMode::PixelRelative);

	auto TestButton = new ToolBarButton();

	TestButton->SetIcon("file:Engine/Editor/Assets/Save.png");
	TestButton->SetName("Save");
	delete TestButton->dropdownButton;

	auto TestButton2 = new ToolBarButton();
	TestButton2->SetName("View");

	ViewportToolbar->AddChild(TestButton);
	ViewportToolbar->AddChild(TestButton2);

	ViewportStatusText = new UIText(12, 0.8f, "", EditorUI::EditorFont);
	ViewportStatusText
		->SetTextSizeMode(UIBox::SizeMode::PixelRelative)
		->SetPadding(4)
		->SetPaddingSizeMode(UIBox::SizeMode::PixelRelative);

	LoadingScreenBox = new UIBackground(true, 0, EditorUI::Theme.Background);

	LoadingScreenBox
		->SetBorder(1, UIBox::SizeMode::PixelRelative)
		->SetBorderColor(EditorUI::Theme.BackgroundHighlight)
		->SetCorner(5, UIBox::SizeMode::PixelRelative)
		->SetVerticalAlign(UIBox::Align::Centered)
		->AddChild((new UISpinner(0, EditorUI::Theme.Highlight1, 32))
			->SetBackgroundColor(EditorUI::Theme.HighlightDark)
			->SetPadding(15, 15, 15, 5)
			->SetPaddingSizeMode(UIBox::SizeMode::PixelRelative))
		->AddChild((new UIBackground(true, 0, EditorUI::Theme.BackgroundHighlight, 1))
			->SetPadding(10)
			->SetPaddingSizeMode(UIBox::SizeMode::PixelRelative)
			->SetSizeMode(UIBox::SizeMode::PixelRelative)
			->SetTryFill(true))
		->AddChild((new UIText(12, EditorUI::Theme.Text, "Loading scene...", EditorUI::EditorFont))
			->SetTextSizeMode(UIBox::SizeMode::PixelRelative)
			->SetPadding(5, 5, 5, 15)
			->SetPaddingSizeMode(UIBox::SizeMode::PixelRelative));

	Background
		->AddChild(ViewportToolbar)
		->AddChild(ViewportBackground
			->AddChild(LoadingScreenBox))
		->AddChild(StatusBarBox
			->AddChild(ViewportStatusText));


	RedrawStats = true;
	CanClose = false;
	Current = this;
}

void engine::editor::Viewport::OnResized()
{
	ViewportBackground->SetMinSize(Background->GetMinSize() - UIBox::PixelSizeToScreenSize(Vec2f(2, 25 + TOOLBAR_SIZE + 2), ViewportBackground->GetParentWindow()));
	PanelElement->UpdateElement();

	VideoSubsystem* VideoSystem = Engine::GetSubsystem<VideoSubsystem>();
	SceneSubsystem* SceneSystem = Engine::GetSubsystem<SceneSubsystem>();

	if (SceneSystem->Main)
		SceneSystem->Main->OnResized(VideoSystem->MainWindow->GetSize());
}

void engine::editor::Viewport::Update()
{
	VideoSubsystem* VideoSystem = Engine::GetSubsystem<VideoSubsystem>();

	Window* Win = VideoSystem->MainWindow;
	FameCount++;

	LoadingScreenBox->IsVisible = !SceneSubsystem::Current->Main;

	if (StatsRedrawTimer.Get() > 1 || RedrawStats)
	{
		SceneSubsystem* SceneSystem = Engine::GetSubsystem<SceneSubsystem>();
		uint64 Fps = uint64(std::round(float(FameCount) / StatsRedrawTimer.Get()));

		int ObjCount = 0;
		string SceneName = "<No name> (Unsaved)";

		if (SceneSystem->Main)
		{
			SceneName = SceneSystem->Main->Name;
			ObjCount = int(SceneSystem->Main->Objects.size());
		}
		ViewportStatusText->SetText(str::Format("Scene: %s | %i Object(s) | %i FPS", SceneName.c_str(), ObjCount, int(Fps)));
		FameCount = 0;
		StatsRedrawTimer.Reset();
		RedrawStats = false;
	}

	if (ViewportBackground == Win->UI.HoveredBox && Win->Input.IsRMBClicked)
	{
		SetFocused();
		MouseGrabbed = true;
		Win->Input.KeyboardFocusInput = false;
		input::ShowMouseCursor = false;
	}

	if (MouseGrabbed && !Win->Input.IsRMBDown)
	{
		input::ShowMouseCursor = true;
		Win->Input.KeyboardFocusInput = true;
		MouseGrabbed = false;
	}
}
#endif