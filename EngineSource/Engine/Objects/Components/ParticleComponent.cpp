#include "ParticleComponent.h"
#include <Engine/File/ModelData.h>
#include <Core/ThreadPool.h>
#include <Engine/Scene.h>
#include <Engine/Stats.h>

engine::ParticleComponent::~ParticleComponent()
{
	Clear();
}

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
	if (!Data)
	{
		return;
	}

	auto CameraView = Transform(RootObject->GetScene()->Graphics.UsedCamera->View).Rotation().Inverse();

	std::lock_guard g{ Data->EmittersMutex };
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
	if (!Data)
	{
		return;
	}

	std::lock_guard g{ Data->EmittersMutex };
	for (auto& i : Data->Emitters)
	{
		std::vector<graphics::Material*> Materials = { i.Data.Material };

		i.InstancesModel->DrawInstanced(Render, In, WorldTransform, From, Materials,
			DrawBoundingBox, DrawStencil, false, i.MainThreadInstanceCount);
	}
}

void engine::ParticleComponent::Load(AssetRef File)
{
	try
	{
		Clear();
		Data = std::make_shared<graphics::ParticleData>(File);

		for (auto& i : Data->Emitters)
		{
			InitializeEmitter(&i);
		}
	}
	catch (SerializeException& e)
	{
		Log::Error(e.what());
	}
}

void engine::ParticleComponent::InitializeEmitter(graphics::ParticleEmitter* Emitter)
{
	auto Render = RootObject->GetScene()->Graphics.Render;

	Emitter->InstancesArray = Render->CreateTransformVertexData(nullptr, 0);
	Emitter->InstancesModel = new graphics::Model(GraphicsModel::Billboard()->Data);

	for (auto& model : Emitter->InstancesModel->ModelVertexBuffers)
	{
		model->AttachArrayData(3, Emitter->InstancesArray, graphics::VertexArrayData::BufferType::PerInstance);
	}
}

void engine::ParticleComponent::Clear()
{
	if (Data)
	{
		for (auto& i : Data->Emitters)
		{
			delete i.InstancesArray;
			delete i.InstancesModel;
			delete i.Data.Material;
		}
	}
	Data = nullptr;
}
