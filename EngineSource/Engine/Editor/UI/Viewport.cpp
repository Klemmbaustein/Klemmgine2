#ifdef EDITOR
#include "Viewport.h"
#include <Engine/Engine.h>
#include <Engine/Subsystem/SceneSubsystem.h>
#include <Engine/Subsystem/VideoSubsystem.h>
#include <Engine/Input.h>
#include "EditorUI.h"
using namespace engine::subsystem;
using namespace kui;

engine::editor::Viewport::Viewport()
	: EditorPanel("Viewport", "viewport")
{
	VideoSubsystem* VideoSystem = Engine::GetSubsystem<VideoSubsystem>();
	
	kui::Shader* HoleShader = VideoSystem->MainWindow->Shaders.LoadShader("shaders/uishader.vert", "ui/ui_hole.frag", "ui hole shader");

	ViewportBackground = new UIBackground(false, 0, 1, 0, HoleShader);
	ViewportBackground
		->SetPadding(1)
		->SetPaddingSizeMode(UIBox::SizeMode::PixelRelative);
	ViewportBackground->HasMouseCollision = true;

	StatusBarBox = new UIBox(true, 0);
	StatusBarBox
		->SetMinSize(Vec2f(0, 23))
		->SetMaxSize(Vec2f(0, 23))
		->SetTryFill(true)
		->SetSizeMode(UIBox::SizeMode::PixelRelative)
		->SetVerticalAlign(UIBox::Align::Centered);

	ViewportStatusText = new UIText(12, 0.8f, "STATUS HERE", EditorUI::EditorFont);
	ViewportStatusText
		->SetTextSizeMode(UIBox::SizeMode::PixelRelative)
		->SetPadding(4)
		->SetPaddingSizeMode(UIBox::SizeMode::PixelRelative);

	Background
		->AddChild(ViewportBackground)
		->AddChild(StatusBarBox
			->AddChild(ViewportStatusText));
	RedrawStats = true;
}

void engine::editor::Viewport::OnResized()
{
	ViewportBackground->SetMinSize(Background->GetMinSize() - UIBox::PixelSizeToScreenSize(Vec2f(2, 25), ViewportBackground->GetParentWindow()));
}

void engine::editor::Viewport::Update()
{
	VideoSubsystem* VideoSystem = Engine::GetSubsystem<VideoSubsystem>();

	Window* Win = VideoSystem->MainWindow;
	FameCount++;
	if (StatsRedrawTimer.Get() > 1 || RedrawStats)
	{
		SceneSubsystem* SceneSystem = Engine::GetSubsystem<SceneSubsystem>();
		uint64 Fps = uint64(std::round(float(FameCount) / StatsRedrawTimer.Get()));
		ViewportStatusText->SetText("Scene: <No name> (Unsaved) | 0 Object(s) | " + std::to_string(Fps) + " FPS");
		FameCount = 0;
		StatsRedrawTimer.Reset();
		RedrawStats = false;
	}

	if (ViewportBackground == Win->UI.HoveredBox && Win->Input.IsRMBClicked)
	{
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