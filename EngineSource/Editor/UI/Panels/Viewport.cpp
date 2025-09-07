#ifdef EDITOR
#include "Viewport.h"
#include <Editor/UI/EditorUI.h>
#include <Editor/UI/Effects/Outline.h>
#include <Editor/UI/Elements/DroppableBox.h>
#include <Engine/Engine.h>
#include <Engine/Graphics/Effects/PostProcess.h>
#include <Engine/Input.h>
#include <Engine/Objects/MeshObject.h>
#include <Engine/Stats.h>
#include <Engine/Subsystem/EditorSubsystem.h>
#include <Engine/Subsystem/VideoSubsystem.h>
#include <kui/UI/UISpinner.h>
using namespace engine::subsystem;
using namespace kui;

engine::editor::Viewport* engine::editor::Viewport::Current = nullptr;

void engine::editor::Viewport::HandleKeyPress(Window* w)
{
	VideoSubsystem* VideoSystem = VideoSubsystem::Current;

	using namespace engine;
	using namespace engine::editor;

	if (!Engine::IsPlaying)
	{
		return;
	}

	if (Viewport::Current->PolledForText)
	{
		return;
	}

	if (Viewport::Current == EditorUI::FocusedPanel)
	{
		if (input::IsKeyDown(input::Key::LSHIFT))
		{
			EditorUI::FocusedPanel = nullptr;
		}
		else
		{
			Engine::GetSubsystem<EditorSubsystem>()->StopProject();
			input::ShowMouseCursor = true;
			Viewport::Current->RedrawStats = true;
			Viewport::Current->SetName(Viewport::Current->UnsavedChanges ? "Viewport*" : "Viewport");
		}
	}
}

engine::editor::Viewport::Viewport()
	: EditorPanel("Viewport", "viewport")
{
	VideoSubsystem* VideoSystem = Engine::GetSubsystem<VideoSubsystem>();

	Shader* HoleShader = VideoSystem->MainWindow->Shaders.LoadShader("shaders/uishader.vert", "ui/ui_hole.frag", "ui hole shader");
	VideoSystem->MainWindow->Input.RegisterOnKeyDownCallback(Key::ESCAPE, &HandleKeyPress);

	ViewportBackground = new UIBackground(false, 0, 1, 0, HoleShader);
	ViewportBackground
		->SetVerticalAlign(UIBox::Align::Centered)
		->SetHorizontalAlign(UIBox::Align::Centered)
		->SetPadding(1_px)
		->SetUpPadding(0);
	ViewportBackground->HasMouseCollision = true;

	StatusBarBox = new UIBox(true, 0);
	StatusBarBox
		->SetMinSize(SizeVec(UISize::Parent(1), 23_px))
		->SetMaxSize(SizeVec(UISize::Parent(1), 23_px))
		->SetVerticalAlign(UIBox::Align::Centered);

	ViewportToolbar = new Toolbar();

	ViewportToolbar->AddButton("Save", "file:Engine/Editor/Assets/Save.png",
		[this]()
		{
			SaveCurrentScene();
		});

	ViewportToolbar->AddDropdown("View", "file:Engine/Editor/Assets/Options.png",
		{
			DropdownMenu::Option{.Name = "Default"},
			DropdownMenu::Option{.Name = "Unlit"},
			DropdownMenu::Option{.Name = "Wireframe"},
			DropdownMenu::Option{.Name = "Collision"}
		});

	ViewportToolbar->AddButton("Run", "file:Engine/Editor/Assets/Run.png",
		std::bind(&Viewport::Run, this));

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

	auto ViewportDropBox = new DroppableBox(false, [this](EditorUI::DraggedItem Item)
		{
			if (!Scene::GetMain())
				return;

			Window* Win = Window::GetActiveWindow();

			auto hit = RayAtCursor();

			Vector3 EndPosition = Vector3::SnapToGrid(hit.ImpactPoint, 0.1f);

			if (Item.ObjectType != 0)
			{
				auto obj = Scene::GetMain()->CreateObjectFromID(Item.ObjectType, EndPosition);
				OnObjectCreated(obj);
				SelectedObjects.clear();
				SelectedObjects.insert(obj);
			}

			AssetRef Dropped = AssetRef::FromPath(Item.Path);

			if (Dropped.Extension != "kmdl")
			{
				return;
			}

			auto obj = Scene::GetMain()->CreateObject<MeshObject>(EndPosition);
			obj->Name = Dropped.DisplayName();
			obj->LoadMesh(Dropped);
			OnObjectCreated(obj);
			SelectedObjects.clear();
			SelectedObjects.insert(obj);
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

	GizmoMesh = new MeshComponent();
	GizmoMesh->Load("Engine/Editor/Assets/Models/Arrows.kmdl"_asset);
	GizmoMesh->Rotation.Y = -90;
}

engine::editor::Viewport::~Viewport()
{
	VideoSubsystem::Current->MainWindow->Input.RemoveOnKeyDownCallback(Key::ESCAPE, &HandleKeyPress);
}

void engine::editor::Viewport::OnResized()
{
	ViewportBackground->SetMinSize(Background->GetMinSize().GetScreen()
		- UIBox::PixelSizeToScreenSize(Vec2f(2.1f, 25 + 42), ViewportBackground->GetParentWindow()));
	PanelElement->UpdateElement();

	VideoSubsystem* VideoSystem = Engine::GetSubsystem<VideoSubsystem>();
	SceneSubsystem* SceneSystem = Engine::GetSubsystem<SceneSubsystem>();

	if (SceneSystem->Main)
		SceneSystem->Main->OnResized(VideoSystem->MainWindow->GetSize());
}

void engine::editor::Viewport::RemoveSelected()
{
	std::vector<SceneObject*> Changed;
	for (auto& i : SelectedObjects)
	{
		Changed.push_back(i);
	}

	OnObjectsChanged(Changed);
	for (auto& i : SelectedObjects)
	{
		i->Destroy();
	}

	SelectedObjects.clear();
}

void engine::editor::Viewport::ClearSelected()
{
	for (auto& i : SelectedObjects)
	{
		HighlightObject(i, false);
	}
	SelectedObjects.clear();
}

void engine::editor::Viewport::Update()
{
	VideoSubsystem* VideoSystem = Engine::GetSubsystem<VideoSubsystem>();

	Window* Win = VideoSystem->MainWindow;
	FameCount++;

	LoadingScreenBox->IsVisible = !SceneSubsystem::Current->Main;
	PolledForText = Win->Input.PollForText;

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

		string ViewportText = str::Format("Scene: %s | %i Object(s) | %i FPS", SceneName.c_str(), ObjCount, int(Fps));

		if (Engine::IsPlaying)
		{
			ViewportText.append(" | Esc: Stop | Shift+Esc: Release mouse cursor");
		}

		ViewportStatusText->SetText(ViewportText);
		FameCount = 0;
		StatsRedrawTimer.Reset();
		RedrawStats = false;
	}
	Scene* Current = Scene::GetMain();

	if (bool(Current) != this->SceneLoaded)
	{
		this->SceneLoaded = bool(Current);
		if (this->SceneLoaded)
		{
			Current->PostProcess.AddEffect(new EditorOutline());
			Current->AddDrawnComponent(GizmoMesh);
		}
	}

	bool HasFocus = EditorUI::FocusedPanel == this;
	UpdateSelection();

	if (Engine::IsPlaying)
	{
		Engine::GameHasFocus = HasFocus;
		input::ShowMouseCursor = HasFocus ? false : true;
		GizmoMesh->IsVisible = false;

		return;
	}
	Engine::GameHasFocus = false;

	if (Current)
	{
		if (SelectedObjects.size())
		{
			GizmoMesh->IsVisible = true;
			GizmoMesh->Position = (*SelectedObjects.begin())->Position;
			GizmoMesh->Scale = Vector3::Distance(Current->UsedCamera->Position, GizmoMesh->Position) * 0.075f;
		}
		else
		{
			GizmoMesh->IsVisible = false;
		}
		GizmoMesh->UpdateTransform();
	}

	if (Current && ViewportBackground == Win->UI.HoveredBox && Win->Input.IsRMBClicked)
	{
		SetFocused();
		MouseGrabbed = true;
		Win->Input.KeyboardFocusInput = false;
		input::ShowMouseCursor = false;
	}

	if (!Current || (MouseGrabbed && !Win->Input.IsRMBDown))
	{
		input::ShowMouseCursor = true;
		Win->Input.KeyboardFocusInput = true;
		MouseGrabbed = false;
	}

	if (input::IsKeyDown(input::Key::F5))
	{
		Run();
	}

	if (input::IsKeyDown(input::Key::LCTRL) && input::IsKeyDown(input::Key::s) && UnsavedChanges && HasFocus)
	{
		SaveCurrentScene();
	}

	if (input::IsKeyDown(input::Key::LCTRL) && input::IsKeyDown(input::Key::z) && HasFocus)
	{
		if (!Undoing)
		{
			UndoLast();
			Undoing = true;
		}
	}
	else
	{
		Undoing = false;
	}

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

		Win->Input.PollForText = false;
		Current->SceneCamera->Rotation = Current->SceneCamera->Rotation + Vector3(-input::MouseMovement.Y, input::MouseMovement.X, 0);
	}
	else if (ViewportBackground == Win->UI.HoveredBox && Current && input::IsLMBClicked)
	{
		Viewport::Current->ClearSelected();
		auto hit = RayAtCursor();
		if (hit.Hit)
		{
			this->SelectedObjects.insert(hit.HitComponent->ParentObject);
		}
	}
}

void engine::editor::Viewport::SceneChanged()
{
	if (Engine::IsPlaying)
	{
		return;
	}

	if (!UnsavedChanges)
	{
		SetName("Viewport*");
	}
	UnsavedChanges = true;
}

void engine::editor::Viewport::SaveCurrentScene()
{
	if (Engine::IsPlaying)
	{
		return;
	}

	Scene* Current = Scene::GetMain();

	if (!Current)
		return;

	Current->Save(Current->Name);
	UnsavedChanges = false;
	SetName("Viewport");
}

void engine::editor::Viewport::OnObjectChanged(SceneObject* Target)
{
	OnObjectsChanged({ Target });
}

void engine::editor::Viewport::OnObjectsChanged(std::vector<SceneObject*> Targets)
{
	Changes NewChanges;

	for (SceneObject* i : Targets)
	{
		NewChanges.ChangeList.push_back(Change{
			.Object = i,
			.ObjectData = i->Serialize()
			});
	}

	ObjectChanges.push(NewChanges);

	SceneChanged();

}
void engine::editor::Viewport::OnObjectCreated(SceneObject* Target)
{
	ObjectChanges.push(Changes{
		.ChangeList = {
			Change{
			.Object = Target,
		}
		} });

	SceneChanged();
}

void engine::editor::Viewport::Run()
{
	using namespace subsystem;

	Engine::GetSubsystem<EditorSubsystem>()->StartProject();
	SetName("Viewport (playing)");
	RedrawStats = true;
}

void engine::editor::Viewport::HighlightObject(SceneObject* Target, bool Highlighted)
{
	for (ObjectComponent* i : Target->GetChildComponents())
	{
		DrawableComponent* Drawable = dynamic_cast<DrawableComponent*>(i);
		if (Drawable)
		{
			HighlightComponents(Drawable, Highlighted);
		}
	}
}

void engine::editor::Viewport::HighlightComponents(DrawableComponent* Target, bool Highlighted)
{
	Target->DrawStencil = Highlighted;
	for (ObjectComponent* i : Target->Children)
	{
		DrawableComponent* Drawable = dynamic_cast<DrawableComponent*>(i);
		if (Drawable)
		{
			HighlightComponents(Drawable, Highlighted);
		}
	}
}

engine::physics::HitResult engine::editor::Viewport::RayAtCursor()
{
	Window* Win = Window::GetActiveWindow();
	graphics::Camera* Cam = Scene::GetMain()->UsedCamera;

	kui::Vec2f MousePos = ((Win->Input.MousePosition - PanelPosition) / Size) * 2 - 1;

	Vector3 Direction = Cam->ScreenToWorld(Vector2(MousePos.X, MousePos.Y));
	Vector3 EndPosition = Cam->Position + Direction * 30;

	auto hit = Scene::GetMain()->Physics.RayCast(Cam->Position, EndPosition, physics::Layer::Dynamic);
	if (!hit.Hit)
	{
		hit.ImpactPoint = Cam->Position + Direction * 10;
	}
	return hit;
}

void engine::editor::Viewport::UndoChange(Change& Target, Scene* Current)
{
	bool Found = false;
	SceneObject* Obj = nullptr;
	for (SceneObject* i : Current->Objects)
	{
		if (i != Target.Object)
		{
			continue;
		}
		Obj = i;
		Found = true;
	}

	try
	{
		if (!Found)
		{
			Obj = Current->CreateObjectFromID(Target.ObjectData.At("typeId").GetInt());
			Obj->DeSerialize(&Target.ObjectData);
			Obj->CheckTransform();
			Obj->CheckComponentTransform();

			EditorUI::SetStatusMessage(str::Format("Undo: Remove %s", Obj->Name.c_str()), EditorUI::StatusType::Info);
		}
		else
		{
			if (Target.ObjectData.IsNull())
			{
				Obj->Destroy();
				EditorUI::SetStatusMessage(str::Format("Undo: Create %s", Obj->Name.c_str()), EditorUI::StatusType::Info);
			}
			else
			{
				Obj->DeSerialize(&Target.ObjectData);
				EditorUI::SetStatusMessage(str::Format("Undo: Modify %s", Obj->Name.c_str()), EditorUI::StatusType::Info);
			}
		}
	}
	catch (SerializeException e)
	{
		Log::Warn(str::Format("Failed to undo change: serialize error: %s", e.what()));
		EditorUI::SetStatusMessage("Undo failed", EditorUI::StatusType::Error);
	}
}

void engine::editor::Viewport::UpdateSelection()
{
	Scene* Current = Scene::GetMain();

	if (!Current)
		return;

	for (SceneObject* i : SelectedObjects)
	{
		bool Found = false;
		for (SceneObject* obj : Current->Objects)
		{
			if (i == obj)
			{
				HighlightObject(obj, true);
				Found = true;
				break;
			}
		}
		if (Found)
			continue;

		SelectedObjects.clear();
		return;
	}
}

void engine::editor::Viewport::UndoLast()
{
	if (ObjectChanges.empty())
	{
		EditorUI::SetStatusMessage("Nothing to undo", EditorUI::StatusType::Warning);
		return;
	}

	Scene* Current = Scene::GetMain();

	auto LastChanges = ObjectChanges.top();

	for (Change& i : LastChanges.ChangeList)
	{
		UndoChange(i, Current);
	}
	ObjectChanges.pop();
}
#endif