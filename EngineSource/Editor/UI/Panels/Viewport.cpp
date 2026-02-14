#include "Viewport.h"
#include <Editor/UI/EditorUI.h>
#include <Editor/UI/Effects/Outline.h>
#include <Editor/UI/Elements/DroppableBox.h>
#include <Editor/UI/Windows/MessageWindow.h>
#include <Engine/Engine.h>
#include <Engine/Graphics/Effects/PostProcess.h>
#include <Engine/Input.h>
#include <Core/File/TextSerializer.h>
#include <Engine/Objects/MeshObject.h>
#include <Engine/Stats.h>
#include <Editor/EditorSubsystem.h>
#include <Engine/Graphics/VideoSubsystem.h>
#include <kui/UI/UISpinner.h>
#include <sstream>
using namespace engine::subsystem;
using namespace kui;
using namespace engine;
using namespace engine::editor;

Viewport* Viewport::Current = nullptr;

engine::editor::Viewport::Viewport()
	: EditorPanel("Viewport", "viewport")
{
	VideoSubsystem* VideoSystem = Engine::GetSubsystem<VideoSubsystem>();

	Shader* HoleShader = VideoSystem->MainWindow->Shaders.LoadShader(
		"shaders/uishader.vert", "ui/ui_hole.frag", "ui hole shader");

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

	ViewportToolbar->AddButton("Save", EditorUI::Asset("Save.png"),
		[this]() {
		SaveCurrentScene();
	});

	ViewportToolbar->AddDropdown("View", EditorUI::Asset("Options.png"), std::bind(&Viewport::GetViewDropdown, this));

	ViewportToolbar->AddButton("Run", EditorUI::Asset("Run.png"),
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
			->SetBackgroundColor(EditorUI::Theme.LightBackground)
			->SetPadding(15_px, 15_px, 15_px, 5_px))
		->AddChild((new UIBackground(true, 0, EditorUI::Theme.BackgroundHighlight, SizeVec(1_px, UISize::Parent(1))))
			->SetPadding(10_px))
		->AddChild((new UIText(12_px, EditorUI::Theme.Text, "Loading scene...", EditorUI::EditorFont))
			->SetPadding(5_px, 5_px, 5_px, 15_px));

	auto ViewportDropBox = new DroppableBox(false, std::bind(&Viewport::OnItemDropped, this, std::placeholders::_1));

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

	auto Win = Window::GetActiveWindow();

	AddShortcut(Key::LEFT, {}, [this, Win] {
		float Speed = (Win->Input.IsKeyDown(Key::CTRL) ? 10 : 1) * GridSize;
		ShiftSelected(Vector3(0, 0, -Speed));
	});

	AddShortcut(Key::RIGHT, {}, [this, Win] {
		float Speed = (Win->Input.IsKeyDown(Key::CTRL) ? 10 : 1) * GridSize;
		ShiftSelected(Vector3(0, 0, Speed));
	});

	AddShortcut(Key::DOWN, {}, [this, Win] {
		bool ShiftDown = Win->Input.IsKeyDown(Key::SHIFT);
		float Speed = (Win->Input.IsKeyDown(Key::CTRL) ? 10 : 1) * GridSize;
		ShiftSelected(ShiftDown ? Vector3(0, -Speed, 0) : Vector3(-Speed, 0, 0));
	});

	AddShortcut(Key::UP, {}, [this, Win] {
		bool ShiftDown = Win->Input.IsKeyDown(Key::SHIFT);
		float Speed = (Win->Input.IsKeyDown(Key::CTRL) ? 10 : 1) * GridSize;
		ShiftSelected(ShiftDown ? Vector3(0, Speed, 0) : Vector3(Speed, 0, 0));
	});

	AddShortcut(Key::c, Key::CTRL, [this, Win] {
		std::stringstream Stream;

		SerializedValue ToCopy = (*SelectedObjects.begin())->Serialize();

		TextSerializer::ToStream(ToCopy.GetObject(), Stream);

		EditorUI::SetStatusMessage("Copied object", EditorUI::StatusType::Info);

		Win->Input.SetClipboard(Stream.str());
	});

	AddShortcut(Key::v, Key::CTRL, [this, Win] {

		std::stringstream Stream;
		Stream << Win->Input.GetClipboard();

		SerializedValue ObjData = TextSerializer::FromStream(Stream);

		auto Obj = Scene::GetMain()->CreateObjectFromID(ObjData.At("typeId").GetInt());
		Obj->DeSerialize(&ObjData);
		Obj->CheckTransform();
		Obj->CheckComponentTransform();

		EditorUI::SetStatusMessage("Pasted object", EditorUI::StatusType::Info);

		ClearSelected();

		SelectedObjects.insert(Obj);

		Win->Input.SetClipboard(Stream.str());
	});

	AddShortcut(Key::s, Key::CTRL, [this] {
		if (!Engine::IsPlaying && UnsavedChanges)
		{
			SaveCurrentScene();
		}
	});

	AddShortcut(Key::z, Key::CTRL, [this] {
		if (!Engine::IsPlaying)
		{
			UndoLast();
		}
	});

	AddShortcut(Key::ESCAPE, {}, [this] {
		if (!Engine::IsPlaying || PolledForText)
		{
			return;
		}
		if (input::IsKeyDown(input::Key::SHIFT))
		{
			EditorUI::FocusedPanel = nullptr;
		}
		else
		{
			Engine::GetSubsystem<EditorSubsystem>()->StopProject();
			input::ShowMouseCursor = true;
			Viewport::Current->RedrawStats = true;
			Viewport::Current->SetName(Viewport::Current->UnsavedChanges
				? "Viewport*" : "Viewport");
		}
	}, ShortcutOptions::AllowInText);

	AddShortcut(Key::F5, {}, [this] {
		if (!Engine::IsPlaying)
		{
			Run();
		}
	}, ShortcutOptions::Global | ShortcutOptions::AllowInText);

	Translate = new TranslateGizmo();

	Grid = new MeshComponent();

	Grid->Load(GraphicsModel::UnitPlane());
	Grid->Materials[0] = new graphics::Material(AssetRef::FromPath(EditorUI::Asset("Models/Grid.kmt")));
	Grid->IsTransparent = true;
	Grid->CastShadow = false;
	Grid->Scale = 1000;
	Grid->UpdateTransform();
}

bool engine::editor::Viewport::GetShowUI()
{
	return this->Visible && (this->ShowUI || Engine::IsPlaying);
}

std::vector<DropdownMenu::Option> engine::editor::Viewport::GetViewDropdown()
{
	return {
		DropdownMenu::Option{
			.Name = "Show UI",
			.Icon = this->ShowUI ? EditorUI::Asset("Dot.png") : "",
			.OnClicked = [this] {
				this->ShowUI = !this->ShowUI;
			},
		},
		DropdownMenu::Option{
			.Name = "Show Grid",
			.Icon = this->ShowGrid ? EditorUI::Asset("Dot.png") : "",
			.OnClicked = [this] {
				this->ShowGrid = !this->ShowGrid;
			},
		}
	};
}

engine::editor::Viewport::~Viewport()
{
	delete Translate;
	delete Grid;
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

	LoadingScreenBox->IsVisible = !SceneSubsystem::Current->Main && SceneSubsystem::Current->IsLoading;
	PolledForText = Win->Input.PollForText;


	if ((StatsRedrawTimer.Get() > 1 || RedrawStats))
	{
		SceneSubsystem* SceneSystem = Engine::GetSubsystem<SceneSubsystem>();
		uint64 Fps = uint64(std::round(float(FameCount) / StatsRedrawTimer.Get()));

		int ObjCount = 0;
		string SceneName = "<No scene>";

		if (SceneSystem->Main)
		{
			SceneName = SceneSystem->Main->Name;
			ObjCount = int(SceneSystem->Main->Objects.size());
		}

		string ViewportText = str::Format("Scene: %s | %i Object(s) | %i FPS", SceneName.c_str(), ObjCount, Fps);

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
			Current->AddDrawnComponent(Grid);
			Current->AddDrawnComponent(Translate->GizmoMesh);
		}
	}

	bool HasFocus = EditorUI::FocusedPanel == this;
	UpdateSelection();

	Grid->IsVisible = !Engine::IsPlaying && ShowGrid;

	if (Engine::IsPlaying)
	{
		Engine::GameHasFocus = HasFocus;
		input::ShowMouseCursor = HasFocus ? false : true;
		Translate->SetVisible(false);

		return;
	}
	Engine::GameHasFocus = false;

	if (Current)
	{
		Current->AlwaysRedraw = this->Visible;

		if (SelectedObjects.size())
		{
			Translate->SetVisible(true);
			Translate->GizmoMesh->Position = (*SelectedObjects.begin())->Position;
			Translate->Collider->Position = (*SelectedObjects.begin())->Position;
			Translate->GizmoMesh->Scale = Vector3::Distance(Current->UsedCamera->Position, Translate->GizmoMesh->Position) * 0.075f;
			Translate->Collider->Scale = Vector3::Distance(Current->UsedCamera->Position, Translate->GizmoMesh->Position) * 0.075f;
		}
		else
		{
			Translate->SetVisible(false);
		}
		if (this->Visible)
		{
			Translate->Update(this);
		}
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

	if (MouseGrabbed && Current)
	{
		float Speed = stats::DeltaTime * 5;

		if (input::IsKeyDown(input::Key::SHIFT))
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
		Current->SceneCamera->Rotation = Current->SceneCamera->Rotation - Vector3(input::MouseMovement.Y, input::MouseMovement.X, 0);
	}
	else if (ViewportBackground == Win->UI.HoveredBox && Current
		&& input::IsLMBClicked && !Translate->HasGrabbedClick)
	{
		if (!input::IsKeyDown(input::Key::SHIFT))
		{
			Viewport::Current->ClearSelected();
		}
		auto hit = RayAtCursor(1000, 0);
		if (hit.Hit)
		{
			this->SelectedObjects.insert(hit.HitComponent->GetRootObject());
		}
	}
}

void engine::editor::Viewport::OnThemeChanged()
{
	ViewportStatusText->SetColor(EditorUI::Theme.Text);
	delete LoadingScreenBox;
	LoadingScreenBox = new UIBackground(true, 0, EditorUI::Theme.Background);

	LoadingScreenBox
		->SetBorder(1_px, EditorUI::Theme.BackgroundHighlight)
		->SetCorner(5_px)
		->SetVerticalAlign(UIBox::Align::Centered)
		->AddChild((new UISpinner(0, EditorUI::Theme.Highlight1, 32_px))
			->SetBackgroundColor(EditorUI::Theme.LightBackground)
			->SetPadding(15_px, 15_px, 15_px, 5_px))
		->AddChild((new UIBackground(true, 0, EditorUI::Theme.BackgroundHighlight, SizeVec(1_px, UISize::Parent(1))))
			->SetPadding(10_px))
		->AddChild((new UIText(12_px, EditorUI::Theme.Text, "Loading scene...", EditorUI::EditorFont))
			->SetPadding(5_px, 5_px, 5_px, 15_px));
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

	BufferStream Buffer;
	Buffer.WriteStringNoNull(Current->SaveToString(Current->Name));
	Buffer.ResetStreamPosition();

	EditorUI::Instance->AssetsProvider->SaveToFile(Current->Name, &Buffer, Buffer.GetSize());

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
			.Object = i->ID,
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
			.Object = Target->ID,
		}
		} });

	SceneChanged();
}

void engine::editor::Viewport::OnItemDropped(EditorUI::DraggedItem Item)
{
	if (!Scene::GetMain())
		return;

	Window* Win = Window::GetActiveWindow();

	auto hit = RayAtCursor(100, 10);

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
		EditorUI::SetStatusMessage("Cannot add an item of type '" + Dropped.Extension + "' into the scene",
			EditorUI::StatusType::Error);
		return;
	}

	auto obj = Scene::GetMain()->CreateObject<MeshObject>(EndPosition);
	obj->Name = Dropped.DisplayName();
	obj->LoadMesh(Dropped);
	OnObjectCreated(obj);
	SelectedObjects.clear();
	SelectedObjects.insert(obj);
}

void engine::editor::Viewport::Run()
{
	using namespace subsystem;

	if (!SceneLoaded)
	{
		new MessageWindow("Cannot run with no scene loaded.", nullptr);
		return;
	}

	Viewport::Current->ClearSelected();
	SetFocused();

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

physics::HitResult engine::editor::Viewport::RayAtCursor(float Distance, float FallbackDistance)
{
	graphics::Camera* Cam = Scene::GetMain()->UsedCamera;

	Vector3 Direction = GetCursorDirection();
	Vector3 EndPosition = Cam->Position + Direction * Distance;

	auto hit = Scene::GetMain()->Physics.RayCast(Cam->Position, EndPosition, physics::Layer::Dynamic
		| physics::Layer::Trigger | physics::Layer::Layer0 | physics::Layer::Layer1 | physics::Layer::Layer2);
	if (!hit.Hit)
	{
		hit.ImpactPoint = Cam->Position + Direction * FallbackDistance;
	}
	return hit;
}

Vector3 engine::editor::Viewport::GetCursorDirection()
{
	Window* Win = Window::GetActiveWindow();
	graphics::Camera* Cam = Scene::GetMain()->UsedCamera;

	Vec2f Pos = ViewportBackground->GetScreenPosition();
	Vec2f Size = ViewportBackground->GetUsedSize().GetScreen();

	Vec2f MousePos = (((Win->Input.MousePosition - Pos) / Size) * 2 - 1).Clamp(-1, 1);

	Vector3 Direction = Cam->ScreenToWorld(Vector2(MousePos.X, MousePos.Y));
	return Direction;
}

void engine::editor::Viewport::ShiftSelected(Vector3 Direction)
{
	if (SelectedObjects.empty())
	{
		return;
	}

	std::vector<SceneObject*> Changed;
	for (auto& i : SelectedObjects)
	{
		i->Position += Direction;
		Changed.push_back(i);
	}

	OnObjectsChanged(Changed);
}

void engine::editor::Viewport::UndoChange(Change& Target, Scene* Current)
{
	bool Found = false;
	SceneObject* Obj = nullptr;
	for (SceneObject* i : Current->Objects)
	{
		if (i->ID != Target.Object)
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

	Changes LastChanges = ObjectChanges.top();

	for (Change& i : LastChanges.ChangeList)
	{
		UndoChange(i, Current);
	}
	ObjectChanges.pop();
}
