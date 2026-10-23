#include "ParticleComponent.h"
#include <Engine/File/ModelData.h>
#include <Core/ThreadPool.h>
#include <Engine/Scene.h>
#include <Engine/Stats.h>

void engine::ParticleComponent::OnAttached()
{
	if (RootObject)
	{
		RootObject->GetScene()->Graphics.AddDrawnComponent(this);
	}
}

void engine::ParticleComponent::OnDetached()
{
	if (RootObject)
	{
		RootObject->GetScene()->Graphics.RemoveDrawnComponent(this);
	}
}

bool engine::ParticleComponent::UpdateTransform(bool IsDirty)
{
	bool Result = DrawableComponent::UpdateTransform(IsDirty);
	if (Data && DrawBoundingBox.Extents == 0 || Result)
	{
		DrawBoundingBox = EmitterBounds.Translate(WorldTransform);
	}
	return Result;
}

void engine::ParticleComponent::Update()
{
	auto CameraView = Transform(RootObject->GetScene()->Graphics.UsedCamera->View).Rotation().Inverse();

	for (auto& i : Data->Emitters)
	{
		if (!i.IsRunning)
		{
			i.IsRunning = true;
			i.MainThreadInstanceCount = i.Transforms.size();
			EmitterBounds = i.Bounds;
			DrawBoundingBox = EmitterBounds.Translate(WorldTransform);
			i.InstancesArray->SetTransformData(i.Transforms.data(), i.Transforms.size());
			ThreadPool::Main()->AddJob([Data = this->Data, &i, CameraView] {
				i.RunStep(stats::DeltaTime, CameraView);
				i.IsRunning = false;
			});
		}
	}
}

void engine::ParticleComponent::Draw(graphics::Renderer* Render, graphics::Camera* From,
	graphics::GraphicsScene* In)
{

	std::vector<graphics::Material*> Materials = { Data->Emitters[0].Data.Material };

	for (auto& i : Data->Emitters)
	{
		i.InstancesModel->DrawInstanced(Render, In, WorldTransform, From, Materials,
			DrawBoundingBox, DrawStencil, false, i.MainThreadInstanceCount);
	}
}

void engine::ParticleComponent::Load(AssetRef File)
{
	Data = std::make_shared<graphics::ParticleData>(File);

	auto Render = RootObject->GetScene()->Graphics.Render;

	for (auto& i : Data->Emitters)
	{
		i.InstancesArray = Render->CreateTransformVertexData(nullptr, 0);
		i.InstancesModel = new graphics::Model(GraphicsModel::Billboard()->Data);

		for (auto& model : i.InstancesModel->ModelVertexBuffers)
		{
			model->AttachArrayData(3, i.InstancesArray, graphics::VertexArrayData::BufferType::PerInstance);
		}
	}
}
