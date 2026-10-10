#include "InstancedMeshComponent.h"
#include <Engine/Objects/SceneObject.h>
#include <Engine/Scene.h>

engine::InstancedMeshComponent::InstancedMeshComponent()
{
	for (size_t i = 0; i < 100000; i++)
	{
		Instances.push_back(Transform(Vector3(std::rand() % 300, std::rand() % 300, std::rand() % 300) - 150, 0, 1));
	}
	InstancedShadows = true;
}

engine::InstancedMeshComponent::~InstancedMeshComponent()
{
	delete InstanceArray;
}

void engine::InstancedMeshComponent::OnAttached()
{
	MeshComponent::OnAttached();

	if (RootObject)
	{
		InstanceArray = RootObject->GetScene()->Graphics.Render->CreateTransformVertexData(Instances.data(), Instances.size());
	}
}

void engine::InstancedMeshComponent::Draw(graphics::Renderer* Render,
	graphics::Camera* From, graphics::GraphicsScene* In)
{
	if (!Attached)
	{
		InitBounds();

		for (auto& model : DrawnModel->Drawable->ModelVertexBuffers)
		{
			model->AttachArrayData(3, InstanceArray, graphics::VertexArrayData::BufferType::PerInstance);
		}
		Attached = true;
	}

	DrawnModel->Drawable->DrawInstanced(Render, In, this->WorldTransform, From, Materials,
		DrawBoundingBox, DrawStencil, IsTransparent, Instances.size());
}

void engine::InstancedMeshComponent::SimpleDraw(graphics::Renderer* Render, graphics::ShaderObject* With)
{
	if (!Attached)
	{
		InitBounds();

		for (auto& model : DrawnModel->Drawable->ModelVertexBuffers)
		{
			model->AttachArrayData(3, InstanceArray, graphics::VertexArrayData::BufferType::PerInstance);
		}
		Attached = true;
	}
	DrawnModel->Drawable->SimpleDrawInstanced(Render, WorldTransform, With, Materials, Instances.size());
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

void engine::InstancedMeshComponent::InitBounds()
{
	Vector3 MinBounds = DrawnModel->Data->Bounds.Position - DrawnModel->Data->Bounds.Extents;
	Vector3 MaxBounds = DrawnModel->Data->Bounds.Position + DrawnModel->Data->Bounds.Extents;

	Vector3 Min = 0;
	Vector3 Max = 0;

	for (auto& i : Instances)
	{
		Vector3 Pos = i.ApplyTo(MinBounds);
		Min = Min.Min(Pos);
		Max = Max.Max(Pos);
	}
	InstancesBounds = BoundingBox::FromMinMax(Min, Max);
}
