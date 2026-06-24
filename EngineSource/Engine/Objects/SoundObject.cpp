#include "SoundObject.h"
#include <Engine/Objects/Components/BillboardComponent.h>
#include <Engine/Objects/Components/PhysicsComponent.h>
#include <Engine/Engine.h>

#if EDITOR
#include <Editor/UI/EditorUI.h>

using namespace engine::editor;
#endif

engine::SoundObject::SoundObject()
{
}

engine::SoundObject::~SoundObject()
{
}

void engine::SoundObject::Begin()
{
	Component = new SoundComponent();
	Attach(Component);

	SoundFile.OnChanged = [this] {
		Component->Load(SoundFile.Value);
		Component->Play(Loop.Value, Is3D.Value);
		Component->SetRange(Range.Value);
		Component->SetVolume(Volume.Value);
		Component->SetPitch(Pitch.Value);
	};

	Loop.OnChanged = [this] {
		Component->Play(Loop.Value, Is3D.Value);
	};

	Is3D.OnChanged = Loop.OnChanged;

	Range.OnChanged = [this] {
		Component->SetRange(Range.Value);
	};

	Volume.OnChanged = [this] {
		Component->SetVolume(Volume.Value);
	};

	Pitch.OnChanged = [this] {
		Component->SetPitch(Pitch.Value);
	};

	Component->Load(SoundFile.Value);
	Component->SetRange(Range.Value);
	Component->SetVolume(Volume.Value);
	Component->SetPitch(Pitch.Value);
	Component->Play(Loop.Value, Is3D.Value);

#if EDITOR

	if (!Engine::IsPlaying)
	{
		auto Billboard = new BillboardComponent();

		Attach(Billboard);
		Billboard->LoadImage(AssetRef::FromPath(EditorUI::Asset("Sound.png")));

		auto Collider = new PhysicsComponent();
		Attach(Collider);
		Collider->CreateSphere(physics::MotionType::Static, physics::Layer::Static, 0.25f);
	}
#endif

}

void engine::SoundObject::OnDestroyed()
{
}

void engine::SoundObject::LoadFile(AssetRef File)
{
	SoundFile.Value = File;
	SoundFile.OnChanged();
}
