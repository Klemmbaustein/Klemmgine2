#include "InstancedMeshComponent.h"
#include <Engine/Objects/SceneObject.h>
#include <Engine/Scene.h>

engine::InstancedMeshComponent::InstancedMeshComponent()
{
	InstancedShadows = true;
}

engine::InstancedMeshComponent::~InstancedMeshComponent()
{
	if (InstanceArray)
	{
		delete InstanceArray;
	}
	if (InstancedModel)
	{
		delete InstancedModel;
	}
}

void engine::InstancedMeshComponent::OnAttached()
{
	MeshComponent::OnAttached();
}

void engine::InstancedMeshComponent::Draw(graphics::Renderer* Render,
	graphics::Camera* From, graphics::GraphicsScene* In)
{
	if (!Attached)
	{
		InitBounds();
		Attached = true;
	}

	InstancedModel->DrawInstanced(Render, In, WorldTransform, From, Materials,
		DrawBoundingBox, DrawStencil, IsTransparent, Instances.size());
}

void engine::InstancedMeshComponent::SimpleDraw(graphics::Renderer* Render, graphics::ShaderObject* With)
{
	if (!Attached)
	{
		InitBounds();
		Attached = true;
	}
	InstancedModel->SimpleDrawInstanced(Render, WorldTransform, With, Materials, Instances.size());
}

bool engine::InstancedMeshComponent::UpdateTransform(bool IsDirty)
{
	bool Result = ObjectComponent::UpdateTransform(IsDirty);
	if (DrawnModel && (DrawBoundingBox.Extents == 0 || Result))
	{
		DrawBoundingBox = InstancesBounds.Translate(WorldTransform);
	}
	return Result;
}

void engine::InstancedMeshComponent::InitializeModel()
{
	if (RootObject)
	{
		InstanceArray = RootObject->GetScene()->Graphics.Render->CreateTransformVertexData(
			Instances.data(), Instances.size());
	}

	InstancedModel = new graphics::Model(DrawnModel->Data);

	for (auto& model : InstancedModel->ModelVertexBuffers)
	{
		model->AttachArrayData(3, InstanceArray, graphics::VertexArrayData::BufferType::PerInstance);
	}

	auto RootScene = GetRootObject()->GetScene();
	RootScene->Graphics.AddDrawnComponent(this);
}

void engine::InstancedMeshComponent::RefreshInstances()
{
	InstanceArray->SetTransformData(Instances.data(), Instances.size());
}

void engine::InstancedMeshComponent::InitBounds()
{
	Vector3 MinBounds = DrawnModel->Data->Bounds.Position - DrawnModel->Data->Bounds.Extents;
	Vector3 MaxBounds = DrawnModel->Data->Bounds.Position + DrawnModel->Data->Bounds.Extents;

	Vector3 Min = 0;
	Vector3 Max = 0;

	for (auto& i : Instances)
	{
		Vector3 MinPos = i.ApplyTo(MinBounds);
		Vector3 MaxPos = i.ApplyTo(MaxBounds);
		Min = Min.Min(MinPos);
		Max = Max.Max(MaxPos);
	}
	InstancesBounds = BoundingBox::FromMinMax(Min, Max);
}
