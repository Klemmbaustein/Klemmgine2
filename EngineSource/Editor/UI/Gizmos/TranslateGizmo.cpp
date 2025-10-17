#include "TranslateGizmo.h"
#include <Editor/UI/Panels/Viewport.h>
#include <Engine/Physics/Physics.h>
#include <kui/Window.h>
#include <Editor/UI/Effects/Outline.h>

using namespace kui;

engine::editor::TranslateGizmo::TranslateGizmo()
{
	GizmoMesh = new MeshComponent();
	GizmoMesh->Load("Engine/Editor/Assets/Models/Arrows.kmdl"_asset);
	GizmoMesh->Rotation.Y = -90;
	GizmoMesh->DrawStencil = true;
	GizmoMesh->CastShadow = false;

	this->Physics.Init();

	Collider = new CollisionComponent();
	Collider->Manager = &this->Physics;
	Collider->Rotation.Y = -90;
	Collider->Load("Engine/Editor/Assets/Models/Arrows.kmdl"_asset);
}

engine::editor::TranslateGizmo::~TranslateGizmo()
{
	delete GizmoMesh;
	delete Collider;
}

void engine::editor::TranslateGizmo::Update(Viewport* With)
{
	GizmoMesh->UpdateTransform();
	Collider->UpdateTransform();
	Collider->Update();

	if (With->SelectedObjects.empty())
	{
		return;
	}

	auto Effect = Scene::GetMain()->PostProcess.GetEffect<EditorOutline>();

	graphics::Camera* Cam = Scene::GetMain()->UsedCamera;

	Vector3 Direction = With->GetCursorDirection();
	Vector3 EndPosition = Cam->Position + Direction * 3000000;
	auto h = Physics.RayCast(Cam->Position, EndPosition, physics::Layer::Static);

	Vector3 Dir = (h.ImpactPoint - GizmoMesh->Position).Normalize();

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
