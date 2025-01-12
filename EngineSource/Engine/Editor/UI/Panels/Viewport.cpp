#ifdef EDITOR
#include "Viewport.h"
#include <Engine/Engine.h>
#include <Engine/Subsystem/SceneSubsystem.h>
#include <Engine/Subsystem/VideoSubsystem.h>
#include <Engine/Subsystem/EditorSubsystem.h>
#include <Engine/Input.h>
#include <kui/UI/UISpinner.h>
#include <Engine/Editor/UI/EditorUI.h>
#include <Engine/Stats.h>
#include <Engine/Editor/UI/Elements/DroppableBox.h>
#include <Toolbar.kui.hpp>
#include <Engine/Objects/MeshObject.h>
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
		->SetPadding(UISize::Pixels(1))
		->SetDownPadding(UISize(0));
	ViewportBackground->HasMouseCollision = true;

	StatusBarBox = new UIBox(true, 0);
	StatusBarBox
		->SetMinSize(SizeVec(UISize::Parent(1), 23_px))
		->SetMaxSize(SizeVec(UISize::Parent(1), 23_px))
		->SetVerticalAlign(UIBox::Align::Centered);

	ViewportToolbar = new UIBackground(true, 0, EditorUI::Theme.Background);
	ViewportToolbar
		->SetBorder(1_px, EditorUI::Theme.BackgroundHighlight)
		->SetBorderEdges(false, true, false, false)
		->SetMinWidth(UISize::Parent(1))
		->SetMinHeight(UISize::Pixels(TOOLBAR_SIZE))
		->SetMaxHeight(UISize::Pixels(TOOLBAR_SIZE))
		->SetPadding(1_px, 0_px, 1_px, 1.1_px);

	auto TestButton = new ToolBarButton();

	TestButton->SetIcon("file:Engine/Editor/Assets/Save.png");
	TestButton->SetName("Save");
	TestButton->btn->OnClicked = [this]()
		{
			Scene* Current = Scene::GetMain();
			if (Current)
				Current->Save(Current->Name);
		};
	delete TestButton->dropdownButton;

	auto TestButton2 = new ToolBarButton();
	TestButton2->SetIcon("file:Engine/Editor/Assets/Options.png");
	TestButton2->SetName("View");

	auto RunButton = new ToolBarButton();
	//RunButton->SetIcon("file:Engine/Editor/Assets/Options.png");
	RunButton->btn->OnClicked = []()
		{
			using namespace subsystem;

			Engine::GetSubsystem<EditorSubsystem>()->StartProject();
		};
	RunButton->SetName("Run");
	delete RunButton->dropdownButton;

	ViewportToolbar->AddChild(TestButton);
	ViewportToolbar->AddChild(TestButton2);
	ViewportToolbar->AddChild(RunButton);

	ViewportStatusText = new UIText(UISize::Pixels(11), EditorUI::Theme.Text, "", EditorUI::EditorFont);
	ViewportStatusText
		->SetPadding(UISize::Pixels(4));

	LoadingScreenBox = new UIBackground(true, 0, EditorUI::Theme.Background);

	LoadingScreenBox
		->SetBorder(1_px, EditorUI::Theme.BackgroundHighlight)
		->SetCorner(5_px)
		->SetVerticalAlign(UIBox::Align::Centered)
		->AddChild((new UISpinner(0, EditorUI::Theme.Highlight1, 32_px))
			->SetBackgroundColor(EditorUI::Theme.HighlightDark)
			->SetPadding(15_px, 15_px, 15_px, 5_px))
		->AddChild((new UIBackground(true, 0, EditorUI::Theme.BackgroundHighlight, SizeVec(1_px, UISize::Parent(1))))
			->SetPadding(10_px))
		->AddChild((new UIText(12_px, EditorUI::Theme.Text, "Loading scene...", EditorUI::EditorFont))
			->SetPadding(5_px, 5_px, 5_px, 15_px));

	auto ViewportDropBox = new DroppableBox(false, [](EditorUI::DraggedItem Item)
		{
			if (Item.ObjectType != 0)
			{
				Scene::GetMain()->CreateObjectFromID(Item.ObjectType);
			}

			AssetRef Dropped = AssetRef::FromPath(Item.Path);

			if (Dropped.Extension != "kmdl")
			{
				return;
			}

			auto obj = Scene::GetMain()->CreateObject<MeshObject>();
			obj->Name = Dropped.DisplayName();
			obj->LoadMesh(Dropped);
		});


	Background
		->AddChild(ViewportToolbar)
		->AddChild(ViewportDropBox
			->AddChild(ViewportBackground
				->AddChild(LoadingScreenBox)))
		->AddChild(StatusBarBox
			->AddChild(ViewportStatusText));


	RedrawStats = true;
	CanClose = false;
	Current = this;
	if (Scene::GetMain())
		Scene::GetMain()->UsedCamera = Scene::GetMain()->SceneCamera;
}

engine::editor::Viewport::~Viewport()
{
}

void engine::editor::Viewport::OnResized()
{
	ViewportBackground->SetMinSize(Background->GetMinSize().GetScreen() - UIBox::PixelSizeToScreenSize(Vec2f(2.1f, 25 + TOOLBAR_SIZE + 2), ViewportBackground->GetParentWindow()));
	PanelElement->UpdateElement();

	VideoSubsystem* VideoSystem = Engine::GetSubsystem<VideoSubsystem>();
	SceneSubsystem* SceneSystem = Engine::GetSubsystem<SceneSubsystem>();

	if (SceneSystem->Main)
		SceneSystem->Main->OnResized(VideoSystem->MainWindow->GetSize());
}

void engine::editor::Viewport::RemoveSelected()
{
	for (auto& i : SelectedObjects)
	{
		i->Destroy();
	}

	SelectedObjects.clear();
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

	Scene* Current = Scene::GetMain();
	if (MouseGrabbed && Current)
	{
		float Speed = stats::DeltaTime * 5;

		if (input::IsKeyDown(input::Key::LSHIFT))
		{
			Speed *= 5;
		}

		if (input::IsKeyDown(input::Key::w))
		{
			Current->SceneCamera->Position += Vector3::Forward(Current->SceneCamera->Rotation) * Speed;
		}
		if (input::IsKeyDown(input::Key::s))
		{
			Current->SceneCamera->Position -= Vector3::Forward(Current->SceneCamera->Rotation) * Speed;
		}
		if (input::IsKeyDown(input::Key::d))
		{
			Current->SceneCamera->Position += Vector3::Right(Current->SceneCamera->Rotation) * Speed;
		}
		if (input::IsKeyDown(input::Key::a))
		{
			Current->SceneCamera->Position -= Vector3::Right(Current->SceneCamera->Rotation) * Speed;
		}

		Current->SceneCamera->Rotation = Current->SceneCamera->Rotation + Vector3(-input::MouseMovement.Y, input::MouseMovement.X, 0);
	}
}
#endif