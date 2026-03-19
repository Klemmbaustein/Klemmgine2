#include "TranslateGizmo.h"
#include <Editor/UI/Panels/Viewport.h>
#include <Engine/Physics/Physics.h>
#include <Editor/UI/EditorUI.h>
#include <kui/Window.h>
#include <Editor/UI/Effects/Outline.h>

using namespace kui;

engine::editor::TranslateGizmo::TranslateGizmo()
{
	GizmoMesh = new MeshComponent();
	GizmoMesh->Load(AssetRef::FromPath(EditorUI::Asset("Models/Arrows.kmdl")), false);
	GizmoMesh->SetRotation(Rotation3(0, -90, 0));
	GizmoMesh->DrawStencil = true;
	GizmoMesh->CastShadow = false;
	GizmoMesh->DrawAsOpaqueStencil = true;

	this->Physics.Init();

	Collider = new CollisionComponent();
	Collider->Manager = &this->Physics;
	Collider->SetRotation(Rotation3(0, -90, 0));
	Collider->Load(AssetRef::FromPath(EditorUI::Asset("Models/Arrows.kmdl")));
}

engine::editor::TranslateGizmo::~TranslateGizmo()
{
	delete GizmoMesh;
	delete Collider;
}

void engine::editor::TranslateGizmo::Update(Viewport* With)
{
	auto Current = Scene::GetMain();
	auto Effect = Current->Graphics.Post.GetEffect<EditorOutline>();

	if (!Effect)
	{
		Effect = new EditorOutline();
		Current->Graphics.AddDrawnComponent(With->Grid);
		Current->Graphics.AddDrawnComponent(GizmoMesh);
		Current->Graphics.Post.AddEffect(Effect);
	}

	if (With->SelectedObjects.empty())
	{
		return;
	}

	GizmoMesh->SetPosition((*With->SelectedObjects.begin())->Position);
	Collider->SetPosition(GizmoMesh->GetPosition());
	GizmoMesh->SetScale(Vector3::Distance(Current->Graphics.UsedCamera->Position,
		GizmoMesh->GetPosition()) * 0.075f);
	Collider->SetScale(GizmoMesh->GetScale());
	GizmoMesh->UpdateTransform(true);
	Collider->UpdateTransform(true);
	Collider->Update();

	graphics::Camera* Cam = Current->Graphics.UsedCamera;

	Vector3 Direction = With->GetCursorDirection();
	Vector3 EndPosition = Cam->Position + Direction * 3000000;
	auto h = Physics.RayCast(Cam->Position, EndPosition, physics::Layer::Static);

	Vector3 Dir = (h.ImpactPoint - GizmoMesh->GetPosition()).Normalize();

	bool Clicked = Window::GetActiveWindow()->Input.IsLMBDown;
	Effect->OutlineShader->Bind();
	Effect->OutlineShader->SetInt(Effect->OutlineShader->GetUniformLocation("u_isHovered"), h.Hit || HasGrabbedClick);

	auto Selected = *With->SelectedObjects.begin();

	if (h.Hit && Clicked && !HasGrabbedClick)
	{
		for (int i = 0; i < 3; i++)
		{
			if (Dir[i] > Dir[(i + 1) % 3] && Dir[i] > Dir[(i + 2) % 3])
			{
				DraggedAxis = i;
				break;
			}
		}
		With->OnObjectChanged(Selected);

		OldNormal = Direction;
		OldDistance = Vector3::Distance(Selected->Position, Cam->Position);
		CurrentPosition = Selected->Position;
		HasGrabbedClick = true;
	}
	else if (HasGrabbedClick)
	{
		CurrentPosition[DraggedAxis] += Direction[DraggedAxis] * OldDistance - OldNormal[DraggedAxis] * OldDistance;
		Selected->Position = Vector3::SnapToGrid(CurrentPosition, With->GridSize);
		OldDistance = Vector3::Distance(CurrentPosition, Cam->Position);
		OldNormal = Direction;
	}
	if (!Clicked && HasGrabbedClick)
	{
		HasGrabbedClick = false;
	}
}
