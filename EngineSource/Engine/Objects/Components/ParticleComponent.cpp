#include "ParticleComponent.h"
#include <Engine/File/ModelData.h>
#include <Core/ThreadPool.h>
#include <Engine/Scene.h>
#include <Engine/Stats.h>
#include <Engine/File/Resource.h>
#include <iostream>

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

void engine::ParticleComponent::Update()
{
	if (!Data)
	{
		return;
	}

	auto CameraView = Transform(RootObject->GetScene()->Graphics.UsedCamera->View).Rotation().Inverse();

	std::lock_guard g{ Data->EmittersMutex };
	std::optional<BoundingBox> NewBounds;
	for (auto& i : Data->Emitters)
	{
		if (!i.IsProcessingEffect)
		{
			i.IsProcessingEffect = true;
			i.MainThreadInstanceCount = i.Transforms.size();
			i.MainThreadBounds = i.Bounds;
			i.InstancesArray->SetTransformData(i.Transforms.data(), i.Transforms.size());
			ThreadPool::Main()->AddJob([Data = this->Data, &i, CameraView] {
				i.RunStep(stats::DeltaTime, CameraView, Data->AutomaticBoundingVolume);
				i.IsProcessingEffect = false;
			});
		}
		if (!NewBounds && i.MainThreadInstanceCount)
		{
			NewBounds = i.MainThreadBounds.Translate(WorldTransform);
		}
		else if (i.MainThreadInstanceCount)
		{
			auto Translated = i.MainThreadBounds.Translate(WorldTransform);
			Vector3 NewMin = (NewBounds->Position - NewBounds->Extents).Min(
				Translated.Position - Translated.Extents);
			Vector3 NewMax = (NewBounds->Position + NewBounds->Extents).Max(
				Translated.Position + Translated.Extents);
			NewBounds = BoundingBox::FromMinMax(NewMin, NewMax);
		}

	}

	if (NewBounds)
	{
		DrawBoundingBox = *NewBounds;
	}
	else
	{
		DrawBoundingBox = {};
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
		if (i.MainThreadInstanceCount)
		{
			std::vector<graphics::Material*> Materials = { i.Data.Material };

			i.InstancesModel->DrawInstanced(Render, In, WorldTransform, From, Materials,
				DrawBoundingBox, DrawStencil, false, i.MainThreadInstanceCount);
		}
	}
}

void engine::ParticleComponent::Load(AssetRef File)
{
	try
	{
		Clear();
		resource::AssetListeners[File.FilePath].Add(this, [this, File] {
			if (DoHotReload)
			{
				Load(File);
			}
		});
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

void engine::ParticleComponent::RemoveEmitter(size_t Index)
{
	auto it = Data->Emitters.begin();

	for (size_t i = 0; i < Index; i++)
	{
		it++;
	}

	auto& e = *it;
	delete e.InstancesArray;
	delete e.InstancesModel;
	delete e.Data.Material;
	Data->Emitters.erase(it);
}

void engine::ParticleComponent::Reset() const
{
	for (auto& i : Data->Emitters)
	{
		i.Reset();
	}
}

bool engine::ParticleComponent::IsPlaying() const
{
	for (auto& i : Data->Emitters)
	{
		if (i.IsPlaying())
		{
			return true;
		}
	}

	return false;
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
	resource::RemoveListener(this);
	Data = nullptr;
}
